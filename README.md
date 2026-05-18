# Thnode: Safety Checks for C/C++ code.

## What is Thnode?

Thnode is a tool that allows developers to add checks for common errors to C/C++ programs.
Errors that would cause a crash or data corruption can be detected and handled.

Errors we check for include:

* NULL pointer dereferences
* Division by zero
* Array underflow

Programmers should be checking for these errors.  Experience shows that they often fail to
do so.  Thnode acts as a second line of defense.

## How is Thnode implemented?

Thnode rewrites your program at build time, adding macros around potentially unsafe
operations, This is implemented as a [clang plugin](https://clang.llvm.org/docs/ClangPlugins.html).

Developers provide implementations of the macros, which check for and deal with errors.
We provide default implementations that log errors and stop the program.

## Example: Reacting to null pointer dereferences

Thnode can instrument your program so that if it dereferences a null pointer, a handler
function is called.

This code:

```C
int accumulate(int *p, int i) {
  *p += i;
}
```

... will be transformed as follows:

```C
int accumulate(int *p, int i) {
  *(NULL_CHECK(p, int)) += i;
}
```

We provide a standard definition of `NULL_CHECK` that aborts the program if the pointer is null:

```C
#define NULL_CHECK(ptrExpr, ptrType) \
  ( (ptrType) my_null_check((void *)ptrExpr, #ptrExpr, __FILE__, __LINE__) )

void *thnode_null_check(void *ptr, const char *expr, const char *file, int line) {
  if (ptr == NULL) {
    fprintf(stderr, "[ERROR] Dereferencing NULL pointer %p in expression %s; at %s:%d\n", ptr, expr, file, line);
    abort();
  }

  return ptr;
}
```

You can easily define your own `NULL_CHECK`.  Suppose your program needs to release some resource on exit.  You can do this with the following code:

```C
// In a header:
#define NULL_CHECK(ptrExpr, ptrType) \
  ( (ptrType)my_custom_null_check((void *)ptrExpr, #ptrExpr, __FILE__, __LINE__) )

// In a .c file:
void *my_custom_null_check(void *ptr, const char *expr, const char *file, int line) {
  if (ptr == NULL) {
    write_log("NULL pointer dereference at %s:%d, gracefully shutting down...\n", file, line);
    release_all_resources_for_shutdown();
    exit(1);
  }

  return ptr;
}

```


## Why not...

### Why not build with a compiler that inserts check for these problems?

The latest versions of GCC and Clang do have features that can insert checks for some of
these issues.  Developers using older compilers (to support older OS versions) don't have
access to these features.

### Why not rewrite all code in a safer language?

If you can do this, you should.  In many cases, rewriting is not feasible.  Large legacy
codebases and third party library code are two common cases where a rewrite is not
feasible.

### Why not tell developers to check for these issues?

We have been doing that for decades.  It helped, but it didn't solve the problem.
We decided to try something else ;)

### Why not add a check for ...

We would like to add more checks over time. Contributions are welcome.

## Dev Loop

```shell
# Build and run tests:
./build.sh && ./test.sh
```

## Contacts

### Mailing list: TBD

### Bug Tracker: TBD
