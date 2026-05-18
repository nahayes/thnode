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

// RUN: %rewrite_c_file %s | tee %t.cpp | grep -v CHECK- | FileCheck %s --check-prefix=CHECK-SOURCE
// RUN: %clang -o %t.exe %t.cpp

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_ACCESS(arr, idx) arr[idx]
#define DENOM_CHECK(denom, denomType) (denom)
#define NULL_CHECK(ptrExpr, ptrType) (ptrExpr)

int lots_of_trouble_in_one_tiny_function(int *arr[], int idx_num, int idx_denom) {
  // Try each thing once:

  // CHECK-SOURCE: int *o0 = ARRAY_ACCESS(arr, idx_num);
  int *o0 = arr[idx_num];

  // CHECK-SOURCE: int o1 = idx_num / DENOM_CHECK( (idx_denom), intmax_t);
  int o1 = idx_num / idx_denom;

  // CHECK-SOURCE: int o2 = *(NULL_CHECK( (o0), int *) );
  int o2 = *o0;

  // CHECK-SOURCE: int o3 = *(NULL_CHECK( ((ARRAY_ACCESS(arr, idx_num / DENOM_CHECK( (idx_denom), intmax_t)))), int *) );
  int o3 = *(arr[idx_num / idx_denom]);

  // CHECK-SOURCE: int o4 = *(NULL_CHECK( (ARRAY_ACCESS(arr, 123 + 45 / DENOM_CHECK( (*(NULL_CHECK( ((ARRAY_ACCESS(arr, idx_num / DENOM_CHECK( (idx_denom), intmax_t)))), int *) )), intmax_t))), int *) );
  int o4 = *arr[123 + 45 / *(arr[idx_num / idx_denom])];

  return o4;
}

// main function to make it build.
int main(void) {
  printf("PASS\n");
  return 0;
}
