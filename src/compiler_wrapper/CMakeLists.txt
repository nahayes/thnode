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

add_executable(compiler_wrapper
    compiler_wrapper_main.cpp
    compiler_command.cpp
)

target_include_directories(compiler_wrapper PRIVATE
    # Added so that any code can include headers from any directory.
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(compiler_wrapper
    util_lib
    compiler_info_lib
)

set_target_properties(compiler_wrapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
