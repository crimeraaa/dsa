#pragma once

#include "common.h"

enum Allocator_Mode {
    ALLOCATOR_MODE_ALLOC,
    ALLOCATOR_MODE_RESIZE,
    ALLOCATOR_MODE_FREE,
};
typedef enum Allocator_Mode Allocator_Mode;

typedef struct Allocator_Args Allocator_Args;
struct Allocator_Args {
    void  *old_ptr;
    size_t old_size;
    size_t new_size;
    size_t alignment;
};

enum Allocator_Error {
    ALLOCATOR_ERROR_NONE,
    ALLOCATOR_ERROR_OUT_OF_MEMORY,
};
typedef enum Allocator_Error Allocator_Error;

typedef struct Allocator Allocator;
struct Allocator {
    void *(*fn)(void *user_ptr, Allocator_Mode mode, Allocator_Args args);
    void   *user_ptr;
};

// A simple wrapper around the `malloc` family.
extern const Allocator GLOBAL_HEAP_ALLOCATOR;

// Panics when an allocation request cannot be fulfilled.
extern const Allocator GLOBAL_PANIC_ALLOCATOR;

// An interface that simply returns `NULL` for all allocation requests.
// This is useful for types that require an allocator interface but can use
// fixed-size memory, e.g. `String_Builder`.
extern const Allocator GLOBAL_NONE_ALLOCATOR;


//=== SINGLE OBJECT MEMORY FUNCTIONS ======================================= {{{

void
mem_free(void *ptr, size_t size, Allocator allocator);

void *
mem_new(size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== ARRAY MEMORY FUNCTIONS =============================================== {{{

void
mem_delete(void *ptr, size_t count, size_t size, Allocator allocator);

void *
mem_make(size_t count, size_t size, size_t align, Allocator allocator);

void *
mem_resize(void *old_ptr, size_t old_count, size_t new_count, size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== RAW MEMORY FUNCTIONS === ============================================= {{{

void
mem_rawfree(void *ptr, size_t size, Allocator allocator);

void *
mem_rawnew(size_t size, size_t align, Allocator allocator);

void *
mem_rawresize(void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator);

//=== }}} ======================================================================

#ifdef ALLOCATOR_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>

static void *
_global_heap_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    unused(user_ptr);
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE:
        // NOTE: If `realloc` fails, `old_ptr` is not freed.
        // Don't free it here, because the caller might still need it!
        return realloc(args.old_ptr, args.new_size);
    case ALLOCATOR_MODE_FREE:
        free(args.old_ptr);
        return NULL;
    default:
        assert(false);
    }
}

static void *
_global_panic_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    unused(user_ptr);
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE: {
        void *new_ptr = realloc(args.old_ptr, args.new_size);
        assert(new_ptr != NULL);
        return new_ptr;
    }
    case ALLOCATOR_MODE_FREE: {
        free(args.old_ptr);
        return NULL;
    }
    default:
        __builtin_unreachable();
    }
}

static void *
_global_none_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    unused(user_ptr);
    unused(args);
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE:
    case ALLOCATOR_MODE_FREE:
        break;
    default:
        __builtin_unreachable();
    }
    return NULL;
}

const Allocator
GLOBAL_HEAP_ALLOCATOR  = {&_global_heap_allocator_fn,  NULL},
GLOBAL_PANIC_ALLOCATOR = {&_global_panic_allocator_fn, NULL},
GLOBAL_NONE_ALLOCATOR  = {&_global_none_allocator_fn,  NULL};

void *
mem_new(size_t size, size_t align, Allocator allocator)
{
    return mem_rawnew(size, align, allocator);
}

void
mem_free(void *ptr, size_t size, Allocator allocator)
{
    mem_rawfree(ptr, size, allocator);
}

void *
mem_make(size_t count, size_t size, size_t align, Allocator allocator)
{
    return mem_rawnew(count * size, align, allocator);
}

void *
mem_resize(void *old_ptr, size_t old_count, size_t new_count, size_t size, size_t align, Allocator allocator)
{
    size_t old_size = old_count * size;
    size_t new_size = new_count * size;
    return mem_rawresize(old_ptr, old_size, new_size, align, allocator);
}

void
mem_delete(void *ptr, size_t count, size_t size, Allocator allocator)
{
    mem_rawfree(ptr, count * size, allocator);
}

void *
mem_rawnew(size_t size, size_t align, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = NULL,
        .old_size   = 0,
        .new_size   = size,
        .alignment  = align,
    };
    return allocator.fn(allocator.user_ptr, ALLOCATOR_MODE_ALLOC, args);
}

void *
mem_rawresize(void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = old_ptr,
        .old_size   = old_size,
        .new_size   = new_size,
        .alignment  = align,
    };
    return allocator.fn(allocator.user_ptr, ALLOCATOR_MODE_RESIZE, args);
}

void
mem_rawfree(void *ptr, size_t size, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = ptr,
        .old_size   = size,
        .new_size   = 0,
        .alignment  = 0,
    };
    allocator.fn(allocator.user_ptr, ALLOCATOR_MODE_FREE, args);
}

#endif // ALLOCATOR_IMPLEMENTATION

// The macro definitions should come AFTER the implementation so that we don't
// have name collisions during function definitions.
#define mem_new(T, allocator)                                                  \
    cast(T *)mem_new(sizeof(T), alignof(T), allocator)

#define mem_free(ptr, allocator)                                               \
    mem_free(ptr, sizeof(*(ptr)), allocator)

#define mem_make(T, count, allocator)                                          \
    cast(T *)mem_make(                                                         \
        count,                                                                 \
        sizeof(T),                                                             \
        alignof(T),                                                            \
        allocator)                                                             \

#define mem_resize(T, old_ptr, old_count, new_count, allocator)                \
    cast(T *)mem_resize(                                                       \
        old_ptr,                                                               \
        old_count,                                                             \
        new_count,                                                             \
        sizeof(T), alignof(T),                                                 \
        allocator)

#define mem_delete(ptr, count, allocator)                                      \
    mem_delete(ptr, count, sizeof(*(ptr)), allocator)
