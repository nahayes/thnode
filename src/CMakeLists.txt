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

cmake_minimum_required(VERSION 3.14)

project(SafetyChecksPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Protobuf REQUIRED)

# Creates _build/compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Include GoogleTest.
#
# The hash in the github URL is the commit for versiuon 1.17.0.
# It was committed on 4/30/2025.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#
# Generate protobuf files
#

# Define the set of proto files.
set(PROTO_FILES
    ${CMAKE_SOURCE_DIR}/proto/config.proto
)

# Proto compiler won't make the output directory. Do so before running protobuf_generate_cpp().
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/proto)

# Generate C++ code for proto files.
protobuf_generate_cpp(
    PROTO_SRCS
    PROTO_HDRS
    ${PROTO_FILES}
    PROTOC_OUT_DIR ${CMAKE_BINARY_DIR}/proto
)

#
# Library of .proto generated code.
#

add_library(config_pb_lib STATIC
    ${PROTO_SRCS}
)
set_target_properties(config_pb_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/proto"
    POSITION_INDEPENDENT_CODE ON
)

#
#
# Compiler Wrapper
#
#

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/compiler_wrapper)

add_executable(compiler_wrapper
    compiler_wrapper/compiler_wrapper_main.cpp
    compiler_wrapper/compiler_command.cpp
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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/compiler_wrapper"
)



#
#
# Utilities
#
#

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/util)

add_library(util_lib STATIC
    util/logging.cpp
    util/path_util.cpp
    util/string_util.cpp
)
target_include_directories(util_lib PRIVATE
    # Added so that any code can include headers from any directory.
    ${CMAKE_SOURCE_DIR}
)

set_target_properties(util_lib PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/util"
)


enable_testing()

add_executable(string_util_test
    util/string_util_test.cpp
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



#
#
# validate_config
#
#

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/validate_config)


#
# Tool used to validate config proto.
#

add_executable(validate_config
    validate_config/validate_config.cpp
)

target_include_directories(validate_config PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(validate_config
    ${Protobuf_LIBRARIES}
    config_pb_lib
)

set_target_properties(validate_config PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/validate_config"
)

#
# compiler_info
#

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/compiler_info)

add_executable(clang_includes
    compiler_info/clang_includes_main.cpp
)

add_library(compiler_info_lib STATIC
    compiler_info/clang_includes.cpp
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



#
# Safety checks plugin.
#

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plugin)

add_library(safety_checks_plugin MODULE
    plugin/safety_checks_plugin.cpp
    plugin/ast_util.cpp
    plugin/ast_visitor.cpp
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
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin"
)


