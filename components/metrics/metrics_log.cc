// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/metrics_log.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/base64.h"
#include "base/basictypes.h"
#include "base/cpu.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram.h"
#include "base/metrics/histogram_samples.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/sys_info.h"
#include "base/time/time.h"
#include "components/metrics/metrics_hashes.h"
#include "components/metrics/metrics_pref_names.h"
#include "components/metrics/metrics_provider.h"
#include "components/metrics/metrics_service_client.h"
#include "components/metrics/proto/histogram_event.pb.h"
#include "components/metrics/proto/system_profile.pb.h"
#include "components/metrics/proto/user_action_event.pb.h"
#include "components/variations/active_field_trials.h"

#if defined(OS_ANDROID)
#include "base/android/build_info.h"
#endif

#if defined(OS_WIN)
#include "base/win/metro.h"

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

using base::SampleCountIterator;
using metrics::HistogramEventProto;
using metrics::ProfilerEventProto;
using metrics::SystemProfileProto;
using metrics::UserActionEventProto;
typedef variations::ActiveGroupId ActiveGroupId;

namespace {

// Any id less than 16 bytes is considered to be a testing id.
bool IsTestingID(const std::string& id) {
  return id.size() < 16;
}

// Returns the date at which the current metrics client ID was created as
// a string containing seconds since the epoch, or "0" if none was found.
std::string GetMetricsEnabledDate(PrefService* pref) {
  if (!pref) {
    NOTREACHED();
    return "0";
  }

  return pref->GetString(metrics::prefs::kMetricsReportingEnabledTimestamp);
}

// Computes a SHA-1 hash of |data| and returns it as a hex string.
std::string ComputeSHA1(const std::string& data) {
  const std::string sha1 = base::SHA1HashString(data);
  return base::HexEncode(sha1.data(), sha1.size());
}

void WriteFieldTrials(const std::vector<ActiveGroupId>& field_trial_ids,
                      SystemProfileProto* system_profile) {
  for (std::vector<ActiveGroupId>::const_iterator it =
       field_trial_ids.begin(); it != field_trial_ids.end(); ++it) {
    SystemProfileProto::FieldTrial* field_trial =
        system_profile->add_field_trial();
    field_trial->set_name_id(it->name);
    field_trial->set_group_id(it->group);
  }
}

// Round a timestamp measured in seconds since epoch to one with a granularity
// of an hour. This can be used before uploaded potentially sensitive
// timestamps.
int64 RoundSecondsToHour(int64 time_in_seconds) {
  return 3600 * (time_in_seconds / 3600);
}

}  // namespace

MetricsLog::MetricsLog(const std::string& client_id,
                       int session_id,
                       LogType log_type,
                       metrics::MetricsServiceClient* client,
                       PrefService* local_state)
    : closed_(false),
      log_type_(log_type),
      client_(client),
      creation_time_(base::TimeTicks::Now()),
      local_state_(local_state) {
  if (IsTestingID(client_id))
    uma_proto_.set_client_id(0);
  else
    uma_proto_.set_client_id(Hash(client_id));

  uma_proto_.set_session_id(session_id);

  SystemProfileProto* system_profile = uma_proto_.mutable_system_profile();
  system_profile->set_build_timestamp(GetBuildTime());
  system_profile->set_app_version(client_->GetVersionString());
  system_profile->set_channel(client_->GetChannel());
}

MetricsLog::~MetricsLog() {
}

// static
void MetricsLog::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterIntegerPref(metrics::prefs::kStabilityLaunchCount, 0);
  registry->RegisterIntegerPref(metrics::prefs::kStabilityCrashCount, 0);
  registry->RegisterIntegerPref(
      metrics::prefs::kStabilityIncompleteSessionEndCount, 0);
  registry->RegisterIntegerPref(
      metrics::prefs::kStabilityBreakpadRegistrationFail, 0);
  registry->RegisterIntegerPref(
      metrics::prefs::kStabilityBreakpadRegistrationSuccess, 0);
  registry->RegisterIntegerPref(metrics::prefs::kStabilityDebuggerPresent, 0);
  registry->RegisterIntegerPref(metrics::prefs::kStabilityDebuggerNotPresent,
                                0);
  registry->RegisterStringPref(metrics::prefs::kStabilitySavedSystemProfile,
                               std::string());
  registry->RegisterStringPref(metrics::prefs::kStabilitySavedSystemProfileHash,
                               std::string());
}

// static
uint64 MetricsLog::Hash(const std::string& value) {
  uint64 hash = metrics::HashMetricName(value);

  // The following log is VERY helpful when folks add some named histogram into
  // the code, but forgot to update the descriptive list of histograms.  When
  // that happens, all we get to see (server side) is a hash of the histogram
  // name.  We can then use this logging to find out what histogram name was
  // being hashed to a given MD5 value by just running the version of Chromium
  // in question with --enable-logging.
  DVLOG(1) << "Metrics: Hash numeric [" << value << "]=[" << hash << "]";

  return hash;
}

// static
int64 MetricsLog::GetBuildTime() {
  static int64 integral_build_time = 0;
  if (!integral_build_time) {
    base::Time time;
    static const char kDateTime[] = __DATE__ " " __TIME__ " GMT";
    bool result = base::Time::FromString(kDateTime, &time);
    DCHECK(result);
    integral_build_time = static_cast<int64>(time.ToTimeT());
  }
  return integral_build_time;
}

// static
int64 MetricsLog::GetCurrentTime() {
  return (base::TimeTicks::Now() - base::TimeTicks()).InSeconds();
}

void MetricsLog::RecordUserAction(const std::string& key) {
  DCHECK(!closed_);

  UserActionEventProto* user_action = uma_proto_.add_user_action_event();
  user_action->set_name_hash(Hash(key));
  user_action->set_time(GetCurrentTime());
}

void MetricsLog::RecordHistogramDelta(const std::string& histogram_name,
                                      const base::HistogramSamples& snapshot) {
  DCHECK(!closed_);
  DCHECK_NE(0, snapshot.TotalCount());

  // We will ignore the MAX_INT/infinite value in the last element of range[].

  HistogramEventProto* histogram_proto = uma_proto_.add_histogram_event();
  histogram_proto->set_name_hash(Hash(histogram_name));
  histogram_proto->set_sum(snapshot.sum());

  for (scoped_ptr<SampleCountIterator> it = snapshot.Iterator(); !it->Done();
       it->Next()) {
    base::Histogram::Sample min;
    base::Histogram::Sample max;
    base::Histogram::Count count;
    it->Get(&min, &max, &count);
    HistogramEventProto::Bucket* bucket = histogram_proto->add_bucket();
    bucket->set_min(min);
    bucket->set_max(max);
    bucket->set_count(count);
  }

  // Omit fields to save space (see rules in histogram_event.proto comments).
  for (int i = 0; i < histogram_proto->bucket_size(); ++i) {
    HistogramEventProto::Bucket* bucket = histogram_proto->mutable_bucket(i);
    if (i + 1 < histogram_proto->bucket_size() &&
        bucket->max() == histogram_proto->bucket(i + 1).min()) {
      bucket->clear_max();
    } else if (bucket->max() == bucket->min() + 1) {
      bucket->clear_min();
    }
  }
}

void MetricsLog::RecordStabilityMetrics(
    const std::vector<metrics::MetricsProvider*>& metrics_providers,
    base::TimeDelta incremental_uptime,
    base::TimeDelta uptime) {
  DCHECK(!closed_);
  DCHECK(HasEnvironment());
  DCHECK(!HasStabilityMetrics());

  PrefService* pref = local_state_;
  DCHECK(pref);

  // Get stability attributes out of Local State, zeroing out stored values.
  // NOTE: This could lead to some data loss if this report isn't successfully
  //       sent, but that's true for all the metrics.

  WriteRequiredStabilityAttributes(pref);

  // Record recent delta for critical stability metrics.  We can't wait for a
  // restart to gather these, as that delay biases our observation away from
  // users that run happily for a looooong time.  We send increments with each
  // uma log upload, just as we send histogram data.
  WriteRealtimeStabilityAttributes(pref, incremental_uptime, uptime);

  SystemProfileProto* system_profile = uma_proto()->mutable_system_profile();
  for (size_t i = 0; i < metrics_providers.size(); ++i)
    metrics_providers[i]->ProvideStabilityMetrics(system_profile);

  // Omit some stats unless this is the initial stability log.
  if (log_type() != INITIAL_STABILITY_LOG)
    return;

  int incomplete_shutdown_count =
      pref->GetInteger(metrics::prefs::kStabilityIncompleteSessionEndCount);
  pref->SetInteger(metrics::prefs::kStabilityIncompleteSessionEndCount, 0);
  int breakpad_registration_success_count =
      pref->GetInteger(metrics::prefs::kStabilityBreakpadRegistrationSuccess);
  pref->SetInteger(metrics::prefs::kStabilityBreakpadRegistrationSuccess, 0);
  int breakpad_registration_failure_count =
      pref->GetInteger(metrics::prefs::kStabilityBreakpadRegistrationFail);
  pref->SetInteger(metrics::prefs::kStabilityBreakpadRegistrationFail, 0);
  int debugger_present_count =
      pref->GetInteger(metrics::prefs::kStabilityDebuggerPresent);
  pref->SetInteger(metrics::prefs::kStabilityDebuggerPresent, 0);
  int debugger_not_present_count =
      pref->GetInteger(metrics::prefs::kStabilityDebuggerNotPresent);
  pref->SetInteger(metrics::prefs::kStabilityDebuggerNotPresent, 0);

  // TODO(jar): The following are all optional, so we *could* optimize them for
  // values of zero (and not include them).
  SystemProfileProto::Stability* stability =
      system_profile->mutable_stability();
  stability->set_incomplete_shutdown_count(incomplete_shutdown_count);
  stability->set_breakpad_registration_success_count(
      breakpad_registration_success_count);
  stability->set_breakpad_registration_failure_count(
      breakpad_registration_failure_count);
  stability->set_debugger_present_count(debugger_present_count);
  stability->set_debugger_not_present_count(debugger_not_present_count);
}

void MetricsLog::RecordGeneralMetrics(
    const std::vector<metrics::MetricsProvider*>& metrics_providers) {
  for (size_t i = 0; i < metrics_providers.size(); ++i)
    metrics_providers[i]->ProvideGeneralMetrics(uma_proto());
}

void MetricsLog::GetFieldTrialIds(
    std::vector<ActiveGroupId>* field_trial_ids) const {
  variations::GetFieldTrialActiveGroupIds(field_trial_ids);
}

bool MetricsLog::HasEnvironment() const {
  return uma_proto()->system_profile().has_uma_enabled_date();
}

bool MetricsLog::HasStabilityMetrics() const {
  return uma_proto()->system_profile().stability().has_launch_count();
}

// The server refuses data that doesn't have certain values.  crashcount and
// launchcount are currently "required" in the "stability" group.
// TODO(isherman): Stop writing these attributes specially once the migration to
// protobufs is complete.
void MetricsLog::WriteRequiredStabilityAttributes(PrefService* pref) {
  int launch_count = pref->GetInteger(metrics::prefs::kStabilityLaunchCount);
  pref->SetInteger(metrics::prefs::kStabilityLaunchCount, 0);
  int crash_count = pref->GetInteger(metrics::prefs::kStabilityCrashCount);
  pref->SetInteger(metrics::prefs::kStabilityCrashCount, 0);

  SystemProfileProto::Stability* stability =
      uma_proto()->mutable_system_profile()->mutable_stability();
  stability->set_launch_count(launch_count);
  stability->set_crash_count(crash_count);
}

void MetricsLog::WriteRealtimeStabilityAttributes(
    PrefService* pref,
    base::TimeDelta incremental_uptime,
    base::TimeDelta uptime) {
  // Update the stats which are critical for real-time stability monitoring.
  // Since these are "optional," only list ones that are non-zero, as the counts
  // are aggregated (summed) server side.

  SystemProfileProto::Stability* stability =
      uma_proto()->mutable_system_profile()->mutable_stability();

  const uint64 incremental_uptime_sec = incremental_uptime.InSeconds();
  if (incremental_uptime_sec)
    stability->set_incremental_uptime_sec(incremental_uptime_sec);
  const uint64 uptime_sec = uptime.InSeconds();
  if (uptime_sec)
    stability->set_uptime_sec(uptime_sec);
}

void MetricsLog::RecordEnvironment(
    const std::vector<metrics::MetricsProvider*>& metrics_providers,
    const std::vector<variations::ActiveGroupId>& synthetic_trials,
    int64 install_date) {
  DCHECK(!HasEnvironment());

  SystemProfileProto* system_profile = uma_proto()->mutable_system_profile();

  std::string brand_code;
  if (client_->GetBrand(&brand_code))
    system_profile->set_brand_code(brand_code);

  int enabled_date;
  bool success =
      base::StringToInt(GetMetricsEnabledDate(local_state_), &enabled_date);
  DCHECK(success);

  // Reduce granularity of the enabled_date field to nearest hour.
  system_profile->set_uma_enabled_date(RoundSecondsToHour(enabled_date));

  // Reduce granularity of the install_date field to nearest hour.
  system_profile->set_install_date(RoundSecondsToHour(install_date));

  system_profile->set_application_locale(client_->GetApplicationLocale());

  SystemProfileProto::Hardware* hardware = system_profile->mutable_hardware();

  // By default, the hardware class is empty (i.e., unknown).
  hardware->set_hardware_class(std::string());

  hardware->set_cpu_architecture(base::SysInfo::OperatingSystemArchitecture());
  hardware->set_system_ram_mb(base::SysInfo::AmountOfPhysicalMemoryMB());
#if defined(OS_WIN)
  hardware->set_dll_base(reinterpret_cast<uint64>(&__ImageBase));
#endif

  SystemProfileProto::OS* os = system_profile->mutable_os();
  std::string os_name = base::SysInfo::OperatingSystemName();
#if defined(OS_WIN)
  // TODO(mad): This only checks whether the main process is a Metro process at
  // upload time; not whether the collected metrics were all gathered from
  // Metro.  This is ok as an approximation for now, since users will rarely be
  // switching from Metro to Desktop mode; but we should re-evaluate whether we
  // can distinguish metrics more cleanly in the future: http://crbug.com/140568
  if (base::win::IsMetroProcess())
    os_name += " (Metro)";
#endif
  os->set_name(os_name);
  os->set_version(base::SysInfo::OperatingSystemVersion());
#if defined(OS_ANDROID)
  os->set_fingerprint(
      base::android::BuildInfo::GetInstance()->android_build_fp());
#endif

  base::CPU cpu_info;
  SystemProfileProto::Hardware::CPU* cpu = hardware->mutable_cpu();
  cpu->set_vendor_name(cpu_info.vendor_name());
  cpu->set_signature(cpu_info.signature());

  std::vector<ActiveGroupId> field_trial_ids;
  GetFieldTrialIds(&field_trial_ids);
  WriteFieldTrials(field_trial_ids, system_profile);
  WriteFieldTrials(synthetic_trials, system_profile);

  for (size_t i = 0; i < metrics_providers.size(); ++i)
    metrics_providers[i]->ProvideSystemProfileMetrics(system_profile);

  std::string serialied_system_profile;
  std::string base64_system_profile;
  if (system_profile->SerializeToString(&serialied_system_profile)) {
    base::Base64Encode(serialied_system_profile, &base64_system_profile);
    PrefService* local_state = local_state_;
    local_state->SetString(metrics::prefs::kStabilitySavedSystemProfile,
                           base64_system_profile);
    local_state->SetString(metrics::prefs::kStabilitySavedSystemProfileHash,
                           ComputeSHA1(serialied_system_profile));
  }
}

bool MetricsLog::LoadSavedEnvironmentFromPrefs() {
  PrefService* local_state = local_state_;
  const std::string base64_system_profile =
      local_state->GetString(metrics::prefs::kStabilitySavedSystemProfile);
  if (base64_system_profile.empty())
    return false;

  const std::string system_profile_hash =
      local_state->GetString(metrics::prefs::kStabilitySavedSystemProfileHash);
  local_state->ClearPref(metrics::prefs::kStabilitySavedSystemProfile);
  local_state->ClearPref(metrics::prefs::kStabilitySavedSystemProfileHash);

  SystemProfileProto* system_profile = uma_proto()->mutable_system_profile();
  std::string serialied_system_profile;
  return base::Base64Decode(base64_system_profile, &serialied_system_profile) &&
         ComputeSHA1(serialied_system_profile) == system_profile_hash &&
         system_profile->ParseFromString(serialied_system_profile);
}

void MetricsLog::CloseLog() {
  DCHECK(!closed_);
  closed_ = true;
}

void MetricsLog::GetEncodedLog(std::string* encoded_log) {
  DCHECK(closed_);
  uma_proto_.SerializeToString(encoded_log);
}
