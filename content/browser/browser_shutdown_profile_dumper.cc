// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/browser_shutdown_profile_dumper.h"

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/debug/trace_event_impl.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "content/public/common/content_switches.h"

namespace content {

BrowserShutdownProfileDumper::BrowserShutdownProfileDumper()
    : blocks_(0),
      dump_file_(NULL) {
}

BrowserShutdownProfileDumper::~BrowserShutdownProfileDumper() {
  WriteTracesToDisc(GetFileName());
}

void BrowserShutdownProfileDumper::WriteTracesToDisc(
    const base::FilePath& file_name) {
  // Note: I have seen a usage of 0.000xx% when dumping - which fits easily.
  // Since the tracer stops when the trace buffer is filled, we'd rather save
  // what we have than nothing since we might see from the amount of events
  // that caused the problem.
  DVLOG(1) << "Flushing shutdown traces to disc. The buffer is %" <<
      base::debug::TraceLog::GetInstance()->GetBufferPercentFull() <<
      " full.";
  DCHECK(!dump_file_);
  dump_file_ = file_util::OpenFile(file_name, "w+");
  if (!IsFileValid()) {
    LOG(ERROR) << "Failed to open performance trace file: " <<
        file_name.value();
    return;
  }
  WriteString("{\"traceEvents\":");
  WriteString("[");

  base::debug::TraceLog::GetInstance()->Flush(
      base::Bind(&BrowserShutdownProfileDumper::WriteTraceDataCollected,
      base::Unretained(this)));

  WriteString("]");
  WriteString("}");
  CloseFile();
}

base::FilePath BrowserShutdownProfileDumper::GetFileName() {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  base::FilePath trace_file =
      command_line.GetSwitchValuePath(switches::kTraceShutdownFile);

  if (!trace_file.empty())
    return trace_file;

  // Default to saving the startup trace into the current dir.
  return base::FilePath().AppendASCII("chrometrace.log");
}

void BrowserShutdownProfileDumper::WriteTraceDataCollected(
    const scoped_refptr<base::RefCountedString>& events_str) {
  if (!IsFileValid())
    return;
  if (blocks_) {
    // Blocks are not comma separated. Beginning with the second block we
    // start therefore to add one in front of the previous block.
    WriteString(",");
  }
  ++blocks_;
  WriteString(events_str->data());
}

bool BrowserShutdownProfileDumper::IsFileValid() {
  return dump_file_ && (ferror(dump_file_) == 0);
}

void BrowserShutdownProfileDumper::WriteString(const std::string& string) {
  WriteChars(string.data(), string.size());
}

void BrowserShutdownProfileDumper::WriteChars(const char* chars, size_t size) {
  if (!IsFileValid())
    return;

  size_t written = fwrite(chars, 1, size, dump_file_);
  if (written != size) {
    LOG(ERROR) << "Error " << ferror(dump_file_) <<
        " in fwrite() to trace file";
    CloseFile();
  }
}

void BrowserShutdownProfileDumper::CloseFile() {
  if (!dump_file_)
    return;
  file_util::CloseFile(dump_file_);
  dump_file_ = NULL;
}

}  // namespace content
