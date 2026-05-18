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

// This file does a bunch of things that programs should not do.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// TODO: null and div0 checks call assert(). They should include assert.h.
#include <assert.h>

void* my_null_check(void *ptr, char *expr, char *file, int line) {
  if (ptr == NULL) {
    fprintf(stderr, "ERROR; for expression %s got NULL pointer; at %s:%d\n", expr, file, line);
    abort();
  }
  return ptr;
}

#define NULL_CHECK(ptrExpr, ptrType) ((ptrType) my_null_check((void*)ptrExpr, #ptrExpr, __FILE__, __LINE__))


// Check for array underflow.
static int check_array_idx_ge_zero(int idx, char* file, int line) {
    if (idx < 0) {
        fprintf(stderr, "ERROR; got array access at index %d; want a value >=0; at %s:%d\n", idx, file, line);
        abort();
    }

    return idx;
}

#define ARRAY_ACCESS(arr, idx) (arr[check_array_idx_ge_zero(idx, __FILE__, __LINE__)])

// Check for division by zero.
int check_denom_not_zero__intmax_t(int denom, const char* denom_expr, const char* file, int line) {
    if (denom == 0) {
        fprintf(stderr, "ERROR; for expression %s got value 0; want a value that is not zero; at %s:%d\n", denom_expr, file, line);
        abort();
    }

    return denom;
}

#define DENOM_CHECK(denom, denomTypeExemplar) (check_denom_not_zero__##denomTypeExemplar(denom, #denom, __FILE__, __LINE__))


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <flag>\n", argv[0]);
        printf("Flags:\n");
        printf("  nullptr - Dereference null pointer\n");
        printf("  div0    - Divide by zero\n");
        return 1;
    }

    if (strcmp(argv[1], "nullptr") == 0) {
        printf("Attempting to dereference null pointer...\n");
        int *ptr = NULL;
        *ptr = 42;  // This will cause a segmentation fault
    }
    else if (strcmp(argv[1], "div0") == 0) {
        printf("Attempting to divide by zero...\n");
        int a = 10;
        int b = 0;
        int result = a / b;  // This will cause a division by zero error
        printf("Result: %d\n", result);  // This line won't be reached
    }
    else if (strcmp(argv[1], "array_idx") == 0) {
        printf("Attempting to access array at negitive index...\n");
        float arr[4] = {1.0, -2.0, 3.0};
        arr[1] = arr[2] - arr[0];  // Harmless.
        assert(arr[1] == 2.0);

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Warray-bounds"
        arr[-1] += 9.1;  // This will cause an array access OOB error.
        #pragma GCC diagnostic pop
    }
    else if (strcmp(argv[1], "overflow") == 0) {
        printf("Attempting to access array out of bounds...\n");
        int arr[5] = {1, 2, 3, 4, 5};
        int index = 10;
        int value = arr[index];  // This will cause a buffer overflow
        printf("Value at index %d: %d\n", index, value);  // This line won't be reached
    }
    else {
        printf("Invalid flag. Use 'nullptr', 'div0', or 'overflow'\n");
        return 1;
    }

    return 0;
}

