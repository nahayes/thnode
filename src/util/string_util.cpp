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

#include "string_util.h"

bool startsWith(const std::string &Str, const std::string &Prefix) {
  if (Str.size() < Prefix.size())
    return false;
  return Str.compare(0, Prefix.size(), Prefix) == 0;
};

bool endsWith(const std::string &Str, const std::string &Suffix) {
  if (Str.size() < Suffix.size())
    return false;
  return Str.compare(Str.size() - Suffix.size(), Suffix.size(), Suffix) == 0;
};

std::vector<std::string> asVectorOfStrings(int Argc, char **Argv) {
  std::vector<std::string> Result;
  Result.reserve(Argc);
  for (int I = 0; I < Argc; I++) {
    Result.push_back(Argv[I]);
  }
  return Result;
}

void replaceStringInPlace(std::string &Str, const std::string &From,
                          const std::string &To) {
  std::string::size_type Pos = 0;
  while ((Pos = Str.find(From, Pos)) != std::string::npos) {
    Str.replace(Pos, From.length(), To);
    Pos += To.length();
  }
}

std::string trimWhitespace(const std::string &Str) {
  static const std::string Whitespace = " \t\r\n";

  std::string Result = Str;
  Result.erase(0, Result.find_first_not_of(Whitespace));
  Result.erase(Result.find_last_not_of(Whitespace) + 1);
  return Result;
}
