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

add_library(util_lib STATIC
    logging.cpp
    path_util.cpp
    string_util.cpp
    temp_dir.cpp
)
target_include_directories(util_lib PRIVATE
    # Added so that any code can include headers from any directory.
    ${CMAKE_SOURCE_DIR}
)

set_target_properties(util_lib PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(string_util_test
    string_util_test.cpp
)

target_include_directories(string_util_test PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(string_util_test
    util_lib
    gtest
    gtest_main
    pthread
)

add_test(NAME string_util_test COMMAND string_util_test)
