# Copyright 2026 the Thnode Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_library(safety_checks_plugin MODULE
    safety_checks_plugin.cpp
    ast_util.cpp
    ast_visitor.cpp
)

target_include_directories(safety_checks_plugin PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}

    # Added so that any code can include headers from any directory.
    ${CMAKE_SOURCE_DIR}

    # Proto compiler generates code.
    # Having the binary output dir in the include path allows code to include generated .pb.h files.
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(safety_checks_plugin PRIVATE
    ${Protobuf_LIBRARIES}
    config_pb_lib
)

set_target_properties(safety_checks_plugin PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
