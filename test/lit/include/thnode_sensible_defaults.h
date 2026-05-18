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

#ifndef THNODE_SENSIBLE_DEFAULTS_H__
#define THNODE_SENSIBLE_DEFAULTS_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// ARRAY_ACCESS

int thnode__check_idx_ge_zero(int idx, const char *arr_expr, const char *idx_expr, const char *file, int line) {
  if (idx < 0) {
    fprintf(stderr, "ERROR; for expression %s[%s] got index value %d; want a value that is not negative; at %s:%d", arr_expr, idx_expr, idx, file, line);
    abort();
  }

  return idx;
}

#define ARRAY_ACCESS(arr, idx) (arr[thnode__check_idx_ge_zero(idx, #arr, #idx, __FILE__, __LINE__)])

// DENOM_CHECK

intmax_t thnode__check_denom_not_zero__intmax_t(intmax_t denom, const char *denom_expr, const char *file, int line) {
  if (denom == 0) {
    fprintf(stderr, "ERROR; for expression %s got value 0; want a value that is not zero; at %s:%d\n", denom_expr, file, line);
    abort();
  }

  return denom;
}

uintmax_t thnode__check_denom_not_zero__uintmax_t(uintmax_t denom, const char *denom_expr, const char *file, int line) {
  if (denom == 0) {
    fprintf(stderr, "ERROR; for expression %s got value 0; want a value that is not zero; at %s:%d\n", denom_expr, file, line);
    abort();
  }

  return denom;
}

long double thnode__check_denom_not_zero__long_double(long double denom, const char *denom_expr, const char *file, int line) {
  if (denom == 0) {
    fprintf(stderr, "ERROR; for expression %s got value 0; want a value that is not zero; at %s:%d\n", denom_expr, file, line);
    abort();
  }

  return denom;
}
#define DENOM_CHECK(denom, denomType) (thnode__check_denom_not_zero__##denomType(denom, #denom, __FILE__, __LINE__))

// NULL_CHECK

void *thnode__null_check(void *ptr, const char *expr, const char *file, int line) {
  if (ptr < (void *)1024) {
    fprintf(stderr, "ERROR; Dereferencing NULLish pointer %p in expression %s; at %s:%d\n", ptr, expr, file, line);
    abort();
  }
  return ptr;
}

#define NULL_CHECK(ptrExpr, ptrType) ((ptrType)thnode__null_check((void *)ptrExpr, #ptrExpr, __FILE__, __LINE__))

#endif // THNODE_SENSIBLE_DEFAULTS_H__
