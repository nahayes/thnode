/*
Copyright 2026 the Thnode Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include "util/logging.h"
#include "util/path_util.h"
#include "util/string_util.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

class CompilerCommand {
public:
  CompilerCommand(int Argc, char **Argv)
      : Argv_(strings::asVectorOfStrings(Argc, Argv)) {}

protected:
  CompilerCommand(std::vector<std::string> Argv,
                  std::vector<std::filesystem::path> FilesWritten)
      : Argv_(std::move(Argv)), FilesWritten_(std::move(FilesWritten)) {}

public:
  friend std::ostream &operator<<(std::ostream &Os, const CompilerCommand &Cmd);

  CompilerCommand withCompilerCommand(const std::string &CompilerToCall) const {
    auto UpdatedArgv = Argv_;
    UpdatedArgv[0] = CompilerToCall;

    return std::move(CompilerCommand(UpdatedArgv, FilesWritten_));
  }

  // Replace source code paths in args list with paths to instrumented files.
  CompilerCommand withSourcePathsRewritten() const;

  // Add an include path so that the given injected header can be #included.
  CompilerCommand
  withIncludeAdded(std::optional<std::string> ThnodeIncludeHeader) const;

  // Construct the commands required to instrument all source files.
  std::vector<CompilerCommand> makeInstrumentationCommands(
      std::optional<std::string> ThnodeIncludeHeader) const;

  // Write the command line this object represents to a strings.
  std::string toString() const;

  const std::string &exePath() const {
    if (Argv_.size() < 1) {
      FATAL << "Compiler command has empty argv_?";
    }
    return Argv_[0];
  }

  const std::vector<std::filesystem::path> &filesWritten() const {
    return FilesWritten_;
  }

protected:
  const std::vector<std::string> Argv_;
  const std::vector<std::filesystem::path> FilesWritten_;
};
