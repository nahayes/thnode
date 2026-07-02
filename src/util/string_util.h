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

#include <string>
#include <vector>

namespace strings {

bool startsWith(const std::string &Str, const std::string &Prefix);

bool endsWith(const std::string &Str, const std::string &Suffix);

std::vector<std::string> asVectorOfStrings(int Argc, char **Argv);

void replaceInPlace(std::string &Str, const std::string &From,
                    const std::string &To);

std::string trimWhitespace(const std::string &Str);

} // namespace strings
