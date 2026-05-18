#!/usr/bin/bash

# Copyright 2026 the Thnode Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eux

# find the source file.
SOURCE_FILE=$1
shift

# where should the altered source be?
mkdir -p _out/tmp
TEMP_DIR=$(mktemp --tmpdir='_out/tmp' -d inst_rewrite.XXXXXX)
REWRITTEN_SOURCE_FILE=${TEMP_DIR}/$(basename "${SOURCE_FILE}")

# Path to the compiled plugin:
PLUGIN_PATH="$PWD/_build/plugin/safety_checks_plugin.so"

if [[ "${SOURCE_FILE}" =~ \.cpp$ ]]; then

  # Some header included by common c++ headers needs this set: ¯\_(ツ)_/¯
  cpp_lib_defines='-D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1'

  # Run clang with our plugin to rewrite the code.
  clang++ -cc1 \
    -load "$PLUGIN_PATH" -plugin safety-checks \
    -plugin-arg-safety-checks --config-string="suppress_line_macros: true" \
    -I /usr/bin/../lib/gcc/aarch64-linux-gnu/11/../../../../include/c++/11 \
    -I /usr/lib/llvm-14/lib/clang/14.0.0/include \
    -I /usr/local/include \
    -I /usr/include/x86_64-linux-gnu \
    -I /usr/include \
    ${cpp_lib_defines} \
    -fcxx-exceptions -fexceptions \
    "$SOURCE_FILE" > "$REWRITTEN_SOURCE_FILE"

elif [[ "${SOURCE_FILE}" =~ \.c$ ]]; then
  # Run clang with our plugin to rewrite the code.
  clang -cc1 \
    -load "$PLUGIN_PATH" -plugin safety-checks \
    -plugin-arg-safety-checks --config-string="suppress_line_macros: true" \
    -I /usr/lib/llvm-14/lib/clang/14.0.0/include \
    -I /usr/local/include \
    -I /usr/include/x86_64-linux-gnu \
    -I /usr/include/linux \
    -I /usr/include \
    -I /usr/lib/llvm-14/lib/clang/14.0.6/include \
    "$SOURCE_FILE" > "$REWRITTEN_SOURCE_FILE"

else
  echo "ERROR: Unknown file extension for ${SOURCE_FILE}" >&2
  exit 1

fi

# Clang failing does not stop the script?
# Test for output file.
if [ ! -f "$REWRITTEN_SOURCE_FILE" ]; then
  echo "ERROR: Output file $REWRITTEN_SOURCE_FILE was not created by clang." >&2
  exit 1
fi

cat "$REWRITTEN_SOURCE_FILE"
