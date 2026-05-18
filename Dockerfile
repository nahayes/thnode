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

# Used to create dev enviornment.

FROM ubuntu:22.04

# Install essential build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    clang \
    llvm \
    llvm-dev \
    libclang-dev \
    clang-format \
    clang-tidy \
    python3 \
    python3-pip \
    python3-venv \
    git \
    emacs \
    nano \
    protobuf-compiler \
    libprotobuf-dev \
    entr \
    ccache \
    gdb \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*  # Cleans up the apt cache to reduce image size

# Don't bother with pipx: We are runing as root in a container, the container is the isolated environment.
RUN pip install lit

# Set working directory
WORKDIR /workspace
