#!/bin/bash

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

# Check if we have enough arguments.
if [ $# -lt 1 ]; then
    echo "Usage: $0 <source-file> [compiler-args...]"
    exit 1
fi
# Get the source file and remove it from arguments
SOURCE_FILE=$1
shift

set -eux

# Create temporary file for rewritten source
TEMP_FILE=$(mktemp /tmp/safety-check.XXXXXX.c)

# Path to the compiled plugin (adjust this to match your container setup)
PLUGIN_PATH="/workspace_SKSKSK/build/safety_checks_plugin.so"

# First run clang with our plugin to rewrite the code.

# Includes generated like so: 
#   $ clang -v -E - < /dev/null 2>&1 | grep '^ '
clang -cc1 -load "$PLUGIN_PATH" -plugin safety-checks \
  -I /usr/lib/llvm-14/lib/clang/14.0.0/include \
  -I /usr/local/include \
  -I /usr/include/aarch64-linux-gnu \
  -I /usr/include \
  "$SOURCE_FILE" > "$TEMP_FILE"

# Add assert.h include if it's not already there
sed -i '1i\#include <assert.h>' "$TEMP_FILE"

# Then compile the rewritten code with gcc
gcc -o "${SOURCE_FILE%.c}" "$TEMP_FILE" "$@"

# Clean up
rm "$TEMP_FILE" 
