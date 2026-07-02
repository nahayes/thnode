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

#
# Tool used to validate config proto.
#

add_executable(validate_config
    validate_config_main.cpp
)

target_include_directories(validate_config PRIVATE
    ${CMAKE_SOURCE_DIR}

    # Proto compiler generates code.
    # Having the binary output dir in the include path allows code to include generated .pb.h files.
    ${CMAKE_BINARY_DIR}

    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(validate_config
    ${Protobuf_LIBRARIES}
    config_pb_lib
)

set_target_properties(validate_config PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
