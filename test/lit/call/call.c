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

// RUN: gcc %s -o %t_noinst.exe
// RUN: %t_noinst.exe | %FileCheckSrc %s --check-prefix=CHECK-NOINST
// RUN: env THNODE_CC=gcc THNODE_CONFIG='wrap_call_re: "fun_.*"' %compiler_driver %s -o %t_inst.exe
// RUN: %t_inst.exe | %FileCheckSrc %s --check-prefix=CHECK-INST

#include <stdio.h>

// Swap fun_1 and fun_2:
#define CALL(fn_name) CALL_IMPL(fn_name)
#define CALL_IMPL(fn) CALL_SWAP_##fn

#define CALL_SWAP_fun_1 fun_2
#define CALL_SWAP_fun_2 fun_1

int fun_1() {
  return 1;
}

int fun_2() {
  return 2;
}

int main() {
  // CHECK-NOINST: Call fun_1; got 1
  // CHECK-INST: Call fun_1; got 2
  printf("Call fun_1; got %d\n", fun_1());
}
