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

add_executable(clang_includes
    clang_includes_main.cpp
)

add_library(compiler_info_lib STATIC
    clang_includes.cpp
)

target_include_directories(compiler_info_lib PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(compiler_info_lib
    util_lib
)

target_include_directories(clang_includes PRIVATE
    # Added so that any code can include headers from any directory.
    ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(clang_includes PRIVATE
    ${LLVM_DEFINITIONS}
)

target_link_libraries(clang_includes
    util_lib
    compiler_info_lib
)
