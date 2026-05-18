/*
Copyright 2026 the Thnode Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// RUN: env THNODE_DEBUG_LEVEL=10 THNODE_CC=g++ %compiler_driver %s -o %t_gcc.exe
// RUN: %t_gcc.exe | FileCheck %s --check-prefix=CHECK-GCC
// RUN: env THNODE_DEBUG_LEVEL=10 THNODE_CC=clang++ %compiler_driver %s -o %t_clang.exe
// RUN: %t_clang.exe | FileCheck %s --check-prefix=CHECK-CLANG

#include <stdio.h>

int main() {
#ifdef __clang__
  // CHECK-CLANG: Compiler: clang
  printf("Compiler: clang\n");
  // Don't check this, it will change:
  printf("Version: %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif __GNUC__ // Defined by both clang and gcc!
  // CHECK-GCC: Compiler: GCC
  printf("Compiler: GCC\n");
  // Don't check this, it will change:
  printf("Version: %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif _MSC_VER
  printf("Compiler: Microsoft Visual C++ (MSVC)\n");
  printf("MSVC Version: %d\n", _MSC_VER);
  return 1; // This is untested, fail until testng is added.
#else
  printf("Compiler: Unknown or other C compiler\n");
  return 1; // Error out; should add to this test if new compilers are supported.
#endif

  return 0;
}
