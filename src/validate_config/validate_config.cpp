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

#include <google/protobuf/text_format.h>

#include <fstream>
#include <iostream>
#include <string>

#include "proto/config.pb.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  // Initialize protobuf
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::ifstream Input(argv[1]);
  if (!Input) {
    std::cerr << "Failed to open input file: " << argv[1] << std::endl;
    return 1;
  }

  // Read the message from the input file.
  thnode::Config Config;
  if (!google::protobuf::TextFormat::ParseFromString(
          std::string((std::istreambuf_iterator<char>(Input)),
                      std::istreambuf_iterator<char>()),
          &Config)) {
    std::cerr << "Failed to parse message from text format" << std::endl;
    return 1;
  }

  // Print the message
  std::cout << "Message contents:" << std::endl;
  std::string ConfigText;
  google::protobuf::TextFormat::PrintToString(Config, &ConfigText);
  std::cout << ConfigText << std::endl;

  // Cleanup
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
