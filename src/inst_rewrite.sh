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

set -euo pipefail

# If any command fails, print an error and exit.
script_abspath="$(readlink -f "$0")"
trap 'echo "ERROR: ${script_abspath}:$LINENO" >&2' ERR

# find the source file.
SOURCE_FILE=$1
shift

# where should the altered source be?
mkdir -p _out/tmp
TEMP_DIR=$(mktemp --tmpdir='_out/tmp' -d inst_rewrite.XXXXXX)
REWRITTEN_SOURCE_FILE=${TEMP_DIR}/$(basename "${SOURCE_FILE}")

# Path to the compiled plugin:
PLUGIN_PATH="$PWD/_build/plugin/safety_checks_plugin.so"
CLANG_INCLUDES_CMD="$PWD/_build/compiler_info/clang_includes"

if [[ "${SOURCE_FILE}" =~ \.cpp$ ]]; then
  COMPILER="clang++"
elif [[ "${SOURCE_FILE}" =~ \.c$ ]]; then
  COMPILER="clang"
else
  echo "ERROR: Unknown file extension for ${SOURCE_FILE}" >&2
  exit 1
fi


clang_abspath=$(which ${COMPILER})
include_paths=$( ${CLANG_INCLUDES_CMD} ${clang_abspath} | sed 's/^/-I /' )

# Some header included by common c++ headers needs this set: ¯\_(ツ)_/¯
cpp_lib_defines='-D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1'

# Run clang with our plugin to rewrite the code.
${COMPILER} -cc1 \
    -load "$PLUGIN_PATH" -plugin safety-checks \
    ${include_paths} \
    ${cpp_lib_defines} \
    -fcxx-exceptions -fexceptions \
    "$SOURCE_FILE" > "$REWRITTEN_SOURCE_FILE"


# Clang failing does not stop the script?
# Test for output file.
if [ ! -f "$REWRITTEN_SOURCE_FILE" ]; then
  echo "ERROR: Output file $REWRITTEN_SOURCE_FILE was not created by clang." >&2
  exit 1
fi

cat "$REWRITTEN_SOURCE_FILE"
