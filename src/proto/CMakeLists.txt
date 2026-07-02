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

#
# Generate protobuf files
#

# Define the set of proto files.
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config.proto
)

# Proto compiler won't make the output directory. Do so before running protobuf_generate_cpp().
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Generate C++ code for proto files.
protobuf_generate_cpp(
    PROTO_SRCS
    PROTO_HDRS
    ${PROTO_FILES}
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Library of .proto generated code.
#

add_library(config_pb_lib STATIC
    ${PROTO_SRCS}
)
set_target_properties(config_pb_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    POSITION_INDEPENDENT_CODE ON
)
