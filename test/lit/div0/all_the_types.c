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

// RUN: env THNODE_DEBUG_LEVEL=10 THNODE_CC=gcc THNODE_INCLUDE_HEADER=%test_root_dir/include/thnode_sensible_defaults.h %compiler_driver %s -o %t_gcc.exe
// RUN: %t_gcc.exe | FileCheck %s --check-prefix=CHECK-EXE
#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int i = (int)2 / (int)1;
  long l = (long)2 / (long)1;

  unsigned u = (unsigned)2 / (unsigned)1;
  unsigned int v = (unsigned int)2 / (unsigned int)1;

  float f = (float)2 / (float)1;
  double d = (double)2 / (double)1;
  long double ld = (long double)2 / (long double)1;

  intmax_t imt = (intmax_t)2 / (intmax_t)1;
  uintmax_t uimt = (uintmax_t)2 / (uintmax_t)1;

  size_t st = (size_t)2 / (size_t)1;
  ssize_t sst = (ssize_t)2 / (ssize_t)1;

  // TODO: More types!

  // CHECK-EXE: PASS
  printf("PASS\n");

  return 0;
}
