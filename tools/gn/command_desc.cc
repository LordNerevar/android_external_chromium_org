// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <set>
#include <sstream>

#include "base/command_line.h"
#include "tools/gn/commands.h"
#include "tools/gn/config.h"
#include "tools/gn/config_values_extractors.h"
#include "tools/gn/filesystem_utils.h"
#include "tools/gn/item.h"
#include "tools/gn/label.h"
#include "tools/gn/setup.h"
#include "tools/gn/standard_out.h"
#include "tools/gn/target.h"

namespace commands {

namespace {

// Prints the given directory in a nice way for the user to view.
std::string FormatSourceDir(const SourceDir& dir) {
#if defined(OS_WIN)
  // On Windows we fix up system absolute paths to look like native ones.
  // Internally, they'll look like "/C:\foo\bar/"
  if (dir.is_system_absolute()) {
    std::string buf = dir.value();
    if (buf.size() > 3 && buf[2] == ':') {
      buf.erase(buf.begin());  // Erase beginning slash.
      ConvertPathToSystem(&buf);  // Convert to backslashes.
      return buf;
    }
  }
#endif
  return dir.value();
}

void RecursiveCollectChildDeps(const Target* target, std::set<Label>* result);

void RecursiveCollectDeps(const Target* target, std::set<Label>* result) {
  if (result->find(target->label()) != result->end())
    return;  // Already did this target.
  result->insert(target->label());

  RecursiveCollectChildDeps(target, result);
}

void RecursiveCollectChildDeps(const Target* target, std::set<Label>* result) {
  const LabelTargetVector& deps = target->deps();
  for (size_t i = 0; i < deps.size(); i++)
    RecursiveCollectDeps(deps[i].ptr, result);

  const LabelTargetVector& datadeps = target->datadeps();
  for (size_t i = 0; i < datadeps.size(); i++)
    RecursiveCollectDeps(datadeps[i].ptr, result);
}

// Prints dependencies of the given target (not the target itself).
void RecursivePrintDeps(const Target* target,
                        const Label& default_toolchain,
                        int indent_level) {
  LabelTargetVector sorted_deps = target->deps();
  const LabelTargetVector& datadeps = target->datadeps();
  sorted_deps.insert(sorted_deps.end(), datadeps.begin(), datadeps.end());
  std::sort(sorted_deps.begin(), sorted_deps.end(),
            LabelPtrLabelLess<Target>());

  std::string indent(indent_level * 2, ' ');
  for (size_t i = 0; i < sorted_deps.size(); i++) {
    OutputString(indent +
        sorted_deps[i].label.GetUserVisibleName(default_toolchain) + "\n");
    RecursivePrintDeps(sorted_deps[i].ptr, default_toolchain, indent_level + 1);
  }
}

void PrintDeps(const Target* target, bool display_header) {
  const CommandLine* cmdline = CommandLine::ForCurrentProcess();
  Label toolchain_label = target->label().GetToolchainLabel();

  // Tree mode is separate.
  if (cmdline->HasSwitch("tree")) {
    if (display_header)
      OutputString("\nDependency tree:\n");
    RecursivePrintDeps(target, toolchain_label, 1);
    return;
  }

  // Collect the deps to display.
  std::vector<Label> deps;
  if (cmdline->HasSwitch("all")) {
    if (display_header)
      OutputString("\nAll recursive dependencies:\n");

    std::set<Label> all_deps;
    RecursiveCollectChildDeps(target, &all_deps);
    for (std::set<Label>::iterator i = all_deps.begin();
         i != all_deps.end(); ++i)
      deps.push_back(*i);
  } else {
    if (display_header) {
      OutputString("\nDirect dependencies "
                   "(try also \"--all\" and \"--tree\"):\n");
    }

    const LabelTargetVector& target_deps = target->deps();
    for (size_t i = 0; i < target_deps.size(); i++)
      deps.push_back(target_deps[i].label);

    const LabelTargetVector& target_datadeps = target->datadeps();
    for (size_t i = 0; i < target_datadeps.size(); i++)
      deps.push_back(target_datadeps[i].label);
  }

  std::sort(deps.begin(), deps.end());
  for (size_t i = 0; i < deps.size(); i++)
    OutputString("  " + deps[i].GetUserVisibleName(toolchain_label) + "\n");
}

// libs and lib_dirs are special in that they're inherited. We don't currently
// implement a blame feature for this since the bottom-up inheritance makes
// this difficult.
void PrintLibDirs(const Target* target, bool display_header) {
  const OrderedSet<SourceDir>& lib_dirs = target->all_lib_dirs();
  if (lib_dirs.empty())
    return;

  if (display_header)
    OutputString("\nlib_dirs\n");

  for (size_t i = 0; i < lib_dirs.size(); i++)
    OutputString("    " + FormatSourceDir(lib_dirs[i]) + "\n");
}

void PrintLibs(const Target* target, bool display_header) {
  const OrderedSet<std::string>& libs = target->all_libs();
  if (libs.empty())
    return;

  if (display_header)
    OutputString("\nlibs\n");

  for (size_t i = 0; i < libs.size(); i++)
    OutputString("    " + libs[i] + "\n");
}

void PrintConfigs(const Target* target, bool display_header) {
  // Configs (don't sort since the order determines how things are processed).
  if (display_header)
    OutputString("\nConfigs (in order applying):\n");

  Label toolchain_label = target->label().GetToolchainLabel();
  const LabelConfigVector& configs = target->configs();
  for (size_t i = 0; i < configs.size(); i++) {
    OutputString("  " +
        configs[i].label.GetUserVisibleName(toolchain_label) + "\n");
  }
}

void PrintSources(const Target* target, bool display_header) {
  if (display_header)
    OutputString("\nSources:\n");

  Target::FileList sources = target->sources();
  std::sort(sources.begin(), sources.end());
  for (size_t i = 0; i < sources.size(); i++)
    OutputString("  " + sources[i].value() + "\n");
}

// Attribute the origin for attributing from where a target came from. Does
// nothing if the input is null or it does not have a location.
void OutputSourceOfDep(const ParseNode* origin, std::ostream& out) {
  if (!origin)
    return;
  Location location = origin->GetRange().begin();
  out << "       (Added by " + location.file()->name().value() << ":"
      << location.line_number() << ")\n";
}

// Templatized writer for writing out different config value types.
template<typename T> struct DescValueWriter {};
template<> struct DescValueWriter<std::string> {
  void operator()(const std::string& str, std::ostream& out) const {
    out << "    " << str << "\n";
  }
};
template<> struct DescValueWriter<SourceFile> {
  void operator()(const SourceFile& file, std::ostream& out) const {
    out << "    " << file.value() << "\n";
  }
};
template<> struct DescValueWriter<SourceDir> {
  void operator()(const SourceDir& dir, std::ostream& out) const {
    out << "    " << FormatSourceDir(dir) << "\n";
  }
};

// Writes a given config value type to the string, optionally with attribution.
// This should match RecursiveTargetConfigToStream in the order it traverses.
template<typename T> void OutputRecursiveTargetConfig(
    const Target* target,
    const char* header_name,
    const std::vector<T>& (ConfigValues::* getter)() const) {
  bool display_blame = CommandLine::ForCurrentProcess()->HasSwitch("blame");

  DescValueWriter<T> writer;
  std::ostringstream out;

  for (ConfigValuesIterator iter(target); !iter.done(); iter.Next()) {
    if ((iter.cur().*getter)().empty())
      continue;

    // Optional blame sub-head.
    if (display_blame) {
      const Config* config = iter.GetCurrentConfig();
      if (config) {
        // Source of this value is a config.
        out << "  From " << config->label().GetUserVisibleName(false) << "\n";
        OutputSourceOfDep(iter.origin(), out);
      } else {
        // Source of this value is the target itself.
        out << "  From " << target->label().GetUserVisibleName(false) << "\n";
      }
    }

    // Actual values.
    ConfigValuesToStream(iter.cur(), getter, writer, out);
  }

  std::string out_str = out.str();
  if (!out_str.empty()) {
    OutputString("\n" + std::string(header_name) + "\n");
    OutputString(out_str);
  }
}

}  // namespace

// desc ------------------------------------------------------------------------

const char kDesc[] = "desc";
const char kDesc_HelpShort[] =
    "desc: Show lots of insightful information about a target.";
const char kDesc_Help[] =
    "gn desc <target label> [<what to show>] [--blame] [--all | --tree]\n"
    "  Displays information about a given labeled target.\n"
    "\n"
    "Possibilities for <what to show>:\n"
    "  (If unspecified an overall summary will be displayed.)\n"
    "\n"
    "  sources\n"
    "      Source files.\n"
    "\n"
    "  configs\n"
    "      Shows configs applied to the given target, sorted in the order\n"
    "      they're specified. This includes both configs specified in the\n"
    "      \"configs\" variable, as well as configs pushed onto this target\n"
    "      via dependencies specifying \"all\" or \"direct\" dependent\n"
    "      configs.\n"
    "\n"
    "  deps [--all | --tree]\n"
    "      Show immediate (or, when \"--all\" or \"--tree\" is specified,\n"
    "      recursive) dependencies of the given target. \"--tree\" shows them\n"
    "      in a tree format.  Otherwise, they will be sorted alphabetically.\n"
    "      Both \"deps\" and \"datadeps\" will be included.\n"
    "\n"
    "  defines       [--blame]\n"
    "  include_dirs  [--blame]\n"
    "  cflags        [--blame]\n"
    "  cflags_cc     [--blame]\n"
    "  cflags_cxx    [--blame]\n"
    "  ldflags       [--blame]\n"
    "  lib_dirs\n"
    "  libs\n"
    "      Shows the given values taken from the target and all configs\n"
    "      applying. See \"--blame\" below.\n"
    "\n"
    "  --blame\n"
    "      Used with any value specified by a config, this will name\n"
    "      the config that specified the value. This doesn't currently work\n"
    "      for libs and lib_dirs because those are inherited and are more\n"
    "      complicated to figure out the blame (patches welcome).\n"
    "\n"
    "Note:\n"
    "  This command will show the full name of directories and source files,\n"
    "  but when directories and source paths are written to the build file,\n"
    "  they will be adjusted to be relative to the build directory. So the\n"
    "  values for paths displayed by this command won't match (but should\n"
    "  mean the same thing.\n"
    "\n"
    "Examples:\n"
    "  gn desc //base:base\n"
    "      Summarizes the given target.\n"
    "\n"
    "  gn desc :base_unittests deps --tree\n"
    "      Shows a dependency tree of the \"base_unittests\" project in\n"
    "      the current directory.\n"
    "\n"
    "  gn desc //base defines --blame\n"
    "      Shows defines set for the //base:base target, annotated by where\n"
    "      each one was set from.\n";

#define OUTPUT_CONFIG_VALUE(name, type) \
    OutputRecursiveTargetConfig<type>(target, #name, &ConfigValues::name);

int RunDesc(const std::vector<std::string>& args) {
  if (args.size() != 1 && args.size() != 2) {
    Err(Location(), "You're holding it wrong.",
        "Usage: \"gn desc <target_name> <what to display>\"").PrintToStdout();
    return 1;
  }

  const Target* target = GetTargetForDesc(args);
  if (!target)
    return 1;

#define CONFIG_VALUE_HANDLER(name, type) \
    } else if (what == #name) { OUTPUT_CONFIG_VALUE(name, type)

  if (args.size() == 2) {
    // User specified one thing to display.
    const std::string& what = args[1];
    if (what == "configs") {
      PrintConfigs(target, false);
    } else if (what == "sources") {
      PrintSources(target, false);
    } else if (what == "deps") {
      PrintDeps(target, false);
    } else if (what == "lib_dirs") {
      PrintLibDirs(target, false);
    } else if (what == "libs") {
      PrintLibs(target, false);

    CONFIG_VALUE_HANDLER(defines, std::string)
    CONFIG_VALUE_HANDLER(include_dirs, SourceDir)
    CONFIG_VALUE_HANDLER(cflags, std::string)
    CONFIG_VALUE_HANDLER(cflags_c, std::string)
    CONFIG_VALUE_HANDLER(cflags_cc, std::string)
    CONFIG_VALUE_HANDLER(cflags_objc, std::string)
    CONFIG_VALUE_HANDLER(cflags_objcc, std::string)
    CONFIG_VALUE_HANDLER(ldflags, std::string)

    } else {
      OutputString("Don't know how to display \"" + what + "\".\n");
      return 1;
    }

#undef CONFIG_VALUE_HANDLER
    return 0;
  }

  // Display summary.

  // Generally we only want to display toolchains on labels when the toolchain
  // is different than the default one for this target (which we always print
  // in the header).
  Label target_toolchain = target->label().GetToolchainLabel();

  // Header.
  OutputString("Target: ", DECORATION_YELLOW);
  OutputString(target->label().GetUserVisibleName(false) + "\n");
  OutputString("Type: ", DECORATION_YELLOW);
  OutputString(std::string(
      Target::GetStringForOutputType(target->output_type())) + "\n");
  OutputString("Toolchain: ", DECORATION_YELLOW);
  OutputString(target_toolchain.GetUserVisibleName(false) + "\n");

  PrintSources(target, true);
  PrintConfigs(target, true);

  OUTPUT_CONFIG_VALUE(defines, std::string)
  OUTPUT_CONFIG_VALUE(include_dirs, SourceDir)
  OUTPUT_CONFIG_VALUE(cflags, std::string)
  OUTPUT_CONFIG_VALUE(cflags_c, std::string)
  OUTPUT_CONFIG_VALUE(cflags_cc, std::string)
  OUTPUT_CONFIG_VALUE(cflags_objc, std::string)
  OUTPUT_CONFIG_VALUE(cflags_objcc, std::string)
  OUTPUT_CONFIG_VALUE(ldflags, std::string)
  PrintLibs(target, true);
  PrintLibDirs(target, true);

  PrintDeps(target, true);

  return 0;
}

}  // namespace commands
