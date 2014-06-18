// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/policy/core/browser/configuration_policy_handler.h"

#include <algorithm>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/prefs/pref_value_map.h"
#include "base/strings/string16.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "components/policy/core/browser/policy_error_map.h"
#include "components/policy/core/common/policy_map.h"
#include "grit/components_strings.h"
#include "url/gurl.h"

namespace policy {

// ConfigurationPolicyHandler implementation -----------------------------------

// static
std::string ConfigurationPolicyHandler::ValueTypeToString(
    base::Value::Type type) {
  static const char* strings[] = {
    "null",
    "boolean",
    "integer",
    "double",
    "string",
    "binary",
    "dictionary",
    "list"
  };
  CHECK(static_cast<size_t>(type) < arraysize(strings));
  return std::string(strings[type]);
}

ConfigurationPolicyHandler::ConfigurationPolicyHandler() {
}

ConfigurationPolicyHandler::~ConfigurationPolicyHandler() {
}

void ConfigurationPolicyHandler::PrepareForDisplaying(
    PolicyMap* policies) const {}

void ConfigurationPolicyHandler::ApplyPolicySettings(
    const policy::PolicyMap& policies,
    PrefValueMap* prefs) {
  NOTREACHED();
}

void ConfigurationPolicyHandler::ApplyPolicySettingsWithParameters(
    const PolicyMap& policies,
    const PolicyHandlerParameters& parameters,
    PrefValueMap* prefs) {
  ApplyPolicySettings(policies, prefs);
}

// TypeCheckingPolicyHandler implementation ------------------------------------

TypeCheckingPolicyHandler::TypeCheckingPolicyHandler(
    const char* policy_name,
    base::Value::Type value_type)
    : policy_name_(policy_name),
      value_type_(value_type) {
}

TypeCheckingPolicyHandler::~TypeCheckingPolicyHandler() {
}

const char* TypeCheckingPolicyHandler::policy_name() const {
  return policy_name_;
}

bool TypeCheckingPolicyHandler::CheckPolicySettings(const PolicyMap& policies,
                                                    PolicyErrorMap* errors) {
  const base::Value* value = NULL;
  return CheckAndGetValue(policies, errors, &value);
}

bool TypeCheckingPolicyHandler::CheckAndGetValue(const PolicyMap& policies,
                                                 PolicyErrorMap* errors,
                                                 const base::Value** value) {
  *value = policies.GetValue(policy_name_);
  if (*value && !(*value)->IsType(value_type_)) {
    errors->AddError(policy_name_,
                     IDS_POLICY_TYPE_ERROR,
                     ValueTypeToString(value_type_));
    return false;
  }
  return true;
}


// IntRangePolicyHandlerBase implementation ------------------------------------

IntRangePolicyHandlerBase::IntRangePolicyHandlerBase(
    const char* policy_name,
    int min,
    int max,
    bool clamp)
    : TypeCheckingPolicyHandler(policy_name, base::Value::TYPE_INTEGER),
      min_(min),
      max_(max),
      clamp_(clamp) {
}

bool IntRangePolicyHandlerBase::CheckPolicySettings(const PolicyMap& policies,
                                                    PolicyErrorMap* errors) {
  const base::Value* value;
  return CheckAndGetValue(policies, errors, &value) &&
      EnsureInRange(value, NULL, errors);
}

IntRangePolicyHandlerBase::~IntRangePolicyHandlerBase() {
}

bool IntRangePolicyHandlerBase::EnsureInRange(const base::Value* input,
                                              int* output,
                                              PolicyErrorMap* errors) {
  if (!input)
    return true;

  int value;
  if (!input->GetAsInteger(&value)) {
    NOTREACHED();
    return false;
  }

  if (value < min_ || value > max_) {
    if (errors) {
      errors->AddError(policy_name(),
                       IDS_POLICY_OUT_OF_RANGE_ERROR,
                       base::IntToString(value));
    }

    if (!clamp_)
      return false;

    value = std::min(std::max(value, min_), max_);
  }

  if (output)
    *output = value;
  return true;
}


// StringMappingListPolicyHandler implementation -----------------------------

StringMappingListPolicyHandler::MappingEntry::MappingEntry(
    const char* policy_value, scoped_ptr<base::Value> map)
    : enum_value(policy_value), mapped_value(map.Pass()) {}

StringMappingListPolicyHandler::MappingEntry::~MappingEntry() {}

StringMappingListPolicyHandler::StringMappingListPolicyHandler(
    const char* policy_name,
    const char* pref_path,
    const GenerateMapCallback& callback)
    : TypeCheckingPolicyHandler(policy_name, base::Value::TYPE_LIST),
      pref_path_(pref_path),
      map_getter_(callback) {}

StringMappingListPolicyHandler::~StringMappingListPolicyHandler() {}

bool StringMappingListPolicyHandler::CheckPolicySettings(
    const PolicyMap& policies,
    PolicyErrorMap* errors) {
  const base::Value* value;
  return CheckAndGetValue(policies, errors, &value) &&
      Convert(value, NULL, errors);
}

void StringMappingListPolicyHandler::ApplyPolicySettings(
    const PolicyMap& policies,
    PrefValueMap* prefs) {
  if (!pref_path_)
    return;
  const base::Value* value = policies.GetValue(policy_name());
  scoped_ptr<base::ListValue> list(new base::ListValue());
  if (value && Convert(value, list.get(), NULL))
    prefs->SetValue(pref_path_, list.release());
}

bool StringMappingListPolicyHandler::Convert(const base::Value* input,
                                             base::ListValue* output,
                                             PolicyErrorMap* errors) {
  if (!input)
    return true;

  const base::ListValue* list_value = NULL;
  if (!input->GetAsList(&list_value)) {
    NOTREACHED();
    return false;
  }

  for (base::ListValue::const_iterator entry(list_value->begin());
       entry != list_value->end(); ++entry) {
    std::string entry_value;
    if (!(*entry)->GetAsString(&entry_value)) {
      if (errors) {
        errors->AddError(policy_name(),
                         entry - list_value->begin(),
                         IDS_POLICY_TYPE_ERROR,
                         ValueTypeToString(base::Value::TYPE_STRING));
      }
      continue;
    }

    scoped_ptr<base::Value> mapped_value = Map(entry_value);
    if (mapped_value) {
      if (output)
        output->Append(mapped_value.release());
    } else {
      if (errors) {
        errors->AddError(policy_name(),
                         entry - list_value->begin(),
                         IDS_POLICY_OUT_OF_RANGE_ERROR);
      }
    }
  }

  return true;
}

scoped_ptr<base::Value> StringMappingListPolicyHandler::Map(
    const std::string& entry_value) {
  // Lazily generate the map of policy strings to mapped values.
  if (map_.empty())
    map_getter_.Run(&map_);

  scoped_ptr<base::Value> return_value;
  for (ScopedVector<MappingEntry>::const_iterator it = map_.begin();
       it != map_.end(); ++it) {
    const MappingEntry* mapping_entry = *it;
    if (mapping_entry->enum_value == entry_value) {
      return_value = make_scoped_ptr(mapping_entry->mapped_value->DeepCopy());
      break;
    }
  }
  return return_value.Pass();
}

// IntRangePolicyHandler implementation ----------------------------------------

IntRangePolicyHandler::IntRangePolicyHandler(const char* policy_name,
                                             const char* pref_path,
                                             int min,
                                             int max,
                                             bool clamp)
    : IntRangePolicyHandlerBase(policy_name, min, max, clamp),
      pref_path_(pref_path) {
}

IntRangePolicyHandler::~IntRangePolicyHandler() {
}

void IntRangePolicyHandler::ApplyPolicySettings(const PolicyMap& policies,
                                                PrefValueMap* prefs) {
  if (!pref_path_)
    return;
  const base::Value* value = policies.GetValue(policy_name());
  int value_in_range;
  if (value && EnsureInRange(value, &value_in_range, NULL)) {
    prefs->SetValue(pref_path_,
                    new base::FundamentalValue(value_in_range));
  }
}


// IntPercentageToDoublePolicyHandler implementation ---------------------------

IntPercentageToDoublePolicyHandler::IntPercentageToDoublePolicyHandler(
    const char* policy_name,
    const char* pref_path,
    int min,
    int max,
    bool clamp)
    : IntRangePolicyHandlerBase(policy_name, min, max, clamp),
      pref_path_(pref_path) {
}

IntPercentageToDoublePolicyHandler::~IntPercentageToDoublePolicyHandler() {
}

void IntPercentageToDoublePolicyHandler::ApplyPolicySettings(
    const PolicyMap& policies,
    PrefValueMap* prefs) {
  if (!pref_path_)
    return;
  const base::Value* value = policies.GetValue(policy_name());
  int percentage;
  if (value && EnsureInRange(value, &percentage, NULL)) {
    prefs->SetValue(pref_path_, base::Value::CreateDoubleValue(
        static_cast<double>(percentage) / 100.));
  }
}


// SimplePolicyHandler implementation ------------------------------------------

SimplePolicyHandler::SimplePolicyHandler(
    const char* policy_name,
    const char* pref_path,
    base::Value::Type value_type)
    : TypeCheckingPolicyHandler(policy_name, value_type),
      pref_path_(pref_path) {
}

SimplePolicyHandler::~SimplePolicyHandler() {
}

void SimplePolicyHandler::ApplyPolicySettings(const PolicyMap& policies,
                                              PrefValueMap* prefs) {
  if (!pref_path_)
    return;
  const base::Value* value = policies.GetValue(policy_name());
  if (value)
    prefs->SetValue(pref_path_, value->DeepCopy());
}


// SchemaValidatingPolicyHandler implementation --------------------------------

SchemaValidatingPolicyHandler::SchemaValidatingPolicyHandler(
    const char* policy_name,
    Schema schema,
    SchemaOnErrorStrategy strategy)
    : policy_name_(policy_name), schema_(schema), strategy_(strategy) {
  DCHECK(schema_.valid());
}

SchemaValidatingPolicyHandler::~SchemaValidatingPolicyHandler() {
}

const char* SchemaValidatingPolicyHandler::policy_name() const {
  return policy_name_;
}

bool SchemaValidatingPolicyHandler::CheckPolicySettings(
    const PolicyMap& policies,
    PolicyErrorMap* errors) {
  const base::Value* value = policies.GetValue(policy_name());
  if (!value)
    return true;

  std::string error_path;
  std::string error;
  bool result = schema_.Validate(*value, strategy_, &error_path, &error);

  if (errors && !error.empty()) {
    if (error_path.empty())
      error_path = "(ROOT)";
    errors->AddError(policy_name_, error_path, error);
  }

  return result;
}

bool SchemaValidatingPolicyHandler::CheckAndGetValue(
    const PolicyMap& policies,
    PolicyErrorMap* errors,
    scoped_ptr<base::Value>* output) {
  const base::Value* value = policies.GetValue(policy_name());
  if (!value)
    return true;

  output->reset(value->DeepCopy());
  std::string error_path;
  std::string error;
  bool result =
      schema_.Normalize(output->get(), strategy_, &error_path, &error, NULL);

  if (errors && !error.empty()) {
    if (error_path.empty())
      error_path = "(ROOT)";
    errors->AddError(policy_name_, error_path, error);
  }

  return result;
}

// SimpleSchemaValidatingPolicyHandler implementation --------------------------

SimpleSchemaValidatingPolicyHandler::SimpleSchemaValidatingPolicyHandler(
    const char* policy_name,
    const char* pref_path,
    Schema schema,
    SchemaOnErrorStrategy strategy,
    RecommendedPermission recommended_permission,
    MandatoryPermission mandatory_permission)
    : SchemaValidatingPolicyHandler(policy_name,
                                    schema.GetKnownProperty(policy_name),
                                    strategy),
      pref_path_(pref_path),
      allow_recommended_(recommended_permission == RECOMMENDED_ALLOWED),
      allow_mandatory_(mandatory_permission == MANDATORY_ALLOWED) {
}

SimpleSchemaValidatingPolicyHandler::~SimpleSchemaValidatingPolicyHandler() {
}

bool SimpleSchemaValidatingPolicyHandler::CheckPolicySettings(
    const PolicyMap& policies,
    PolicyErrorMap* errors) {
  const PolicyMap::Entry* policy_entry = policies.Get(policy_name());
  if (!policy_entry)
    return true;
  if ((policy_entry->level == policy::POLICY_LEVEL_MANDATORY &&
       !allow_mandatory_) ||
      (policy_entry->level == policy::POLICY_LEVEL_RECOMMENDED &&
       !allow_recommended_)) {
    if (errors)
      errors->AddError(policy_name(), IDS_POLICY_LEVEL_ERROR);
    return false;
  }

  return SchemaValidatingPolicyHandler::CheckPolicySettings(policies, errors);
}

void SimpleSchemaValidatingPolicyHandler::ApplyPolicySettings(
    const PolicyMap& policies,
    PrefValueMap* prefs) {
  if (!pref_path_)
    return;
  const base::Value* value = policies.GetValue(policy_name());
  if (value)
    prefs->SetValue(pref_path_, value->DeepCopy());
}

// LegacyPoliciesDeprecatingPolicyHandler implementation -----------------------

// TODO(binjin): Add a new common base class for SchemaValidatingPolicyHandler
// and TypeCheckingPolicyHandler representing policy handlers for a single
// policy, and use it as the type of |new_policy_handler|.
// http://crbug.com/345299
LegacyPoliciesDeprecatingPolicyHandler::LegacyPoliciesDeprecatingPolicyHandler(
    ScopedVector<ConfigurationPolicyHandler> legacy_policy_handlers,
    scoped_ptr<SchemaValidatingPolicyHandler> new_policy_handler)
    : legacy_policy_handlers_(legacy_policy_handlers.Pass()),
      new_policy_handler_(new_policy_handler.Pass()) {
}

LegacyPoliciesDeprecatingPolicyHandler::
    ~LegacyPoliciesDeprecatingPolicyHandler() {
}

bool LegacyPoliciesDeprecatingPolicyHandler::CheckPolicySettings(
    const PolicyMap& policies,
    PolicyErrorMap* errors) {
  if (policies.Get(new_policy_handler_->policy_name())) {
    return new_policy_handler_->CheckPolicySettings(policies, errors);
  } else {
    // The new policy is not set, fall back to legacy ones.
    ScopedVector<ConfigurationPolicyHandler>::iterator handler;
    bool valid_policy_found = false;
    for (handler = legacy_policy_handlers_.begin();
         handler != legacy_policy_handlers_.end();
         ++handler) {
      if ((*handler)->CheckPolicySettings(policies, errors))
        valid_policy_found = true;
    }
    return valid_policy_found;
  }
}

void LegacyPoliciesDeprecatingPolicyHandler::ApplyPolicySettings(
    const PolicyMap& policies,
    PrefValueMap* prefs) {
  if (policies.Get(new_policy_handler_->policy_name())) {
    new_policy_handler_->ApplyPolicySettings(policies, prefs);
  } else {
    // The new policy is not set, fall back to legacy ones.
    PolicyErrorMap scoped_errors;
    ScopedVector<ConfigurationPolicyHandler>::iterator handler;
    for (handler = legacy_policy_handlers_.begin();
         handler != legacy_policy_handlers_.end();
         ++handler) {
      if ((*handler)->CheckPolicySettings(policies, &scoped_errors))
        (*handler)->ApplyPolicySettings(policies, prefs);
    }
  }
}

}  // namespace policy
