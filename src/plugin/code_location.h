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

#include "llvm/Support/raw_ostream.h"
class CodeLocation {
public:
  CodeLocation(const char *File, const int Line) : File_(File), Line_(Line) {}

  // Support printing CodeLocation to llvm::raw_ostream (for use with
  // llvm::errs())
  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const CodeLocation &Loc) {
    return OS << Loc.File_ << ":" << Loc.Line_;
  }

private:
  const char *File_;
  const int Line_;
};

#define HERE (CodeLocation(__FILE__, __LINE__))
