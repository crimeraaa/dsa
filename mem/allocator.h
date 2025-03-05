#pragma once

#include "../common.h"

enum Allocator_Mode {
    ALLOCATOR_MODE_ALLOC,
    ALLOCATOR_MODE_RESIZE,
    ALLOCATOR_MODE_FREE,
    ALLOCATOR_MODE_FREE_ALL,
};
typedef enum Allocator_Mode Allocator_Mode;

typedef struct Allocator_Args Allocator_Args;
struct Allocator_Args {
    void  *old_ptr;
    size_t old_size;
    size_t new_size;
    size_t alignment;
};

// The zero-value `ALLOCATOR_ERROR_NONE` indicates success while nonzero indicates
// some failure. You can simply check `if (error)`.
enum Allocator_Error {
    ALLOCATOR_ERROR_NONE,                   // Indicates success.
    ALLOCATOR_ERROR_OUT_OF_MEMORY,
    ALLOCATOR_ERROR_MODE_NOT_IMPLEMENTED,   // e.g. calling `free_all` on the global heap allocator.
};
typedef enum Allocator_Error Allocator_Error;

typedef struct Allocator Allocator;
struct Allocator {
    void *(*fn)(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args);
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

Allocator_Error
mem_free(void *ptr, size_t size, Allocator allocator);

void *
mem_new(Allocator_Error *out_error, size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== ARRAY MEMORY FUNCTIONS =============================================== {{{

Allocator_Error
mem_delete(void *ptr, size_t count, size_t size, Allocator allocator);

void *
mem_make(Allocator_Error *out_error, size_t count, size_t size, size_t align, Allocator allocator);

void *
mem_resize(Allocator_Error *out_error, void *old_ptr, size_t old_count, size_t new_count, size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== RAW MEMORY FUNCTIONS === ============================================= {{{

Allocator_Error
mem_rawfree(void *ptr, size_t size, Allocator allocator);

void *
mem_rawnew(Allocator_Error *out_error, size_t size, size_t align, Allocator allocator);

void *
mem_rawresize(Allocator_Error *out_error, void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator);

//=== }}} ======================================================================

Allocator_Error
mem_free_all(Allocator allocator);

#ifdef ALLOCATOR_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>

static void *
_global_heap_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = ALLOCATOR_ERROR_NONE;
    unused(user_ptr);
    void *data = NULL;
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE:
        // NOTE: If `realloc` fails, `old_ptr` is not freed.
        // Don't free it here, because the caller might still need it!
        data = realloc(args.old_ptr, args.new_size);
        if (data == NULL)
            *out_error = ALLOCATOR_ERROR_OUT_OF_MEMORY;
        break;

    case ALLOCATOR_MODE_FREE:
        free(args.old_ptr);
        break;

    case ALLOCATOR_MODE_FREE_ALL:
        *out_error = ALLOCATOR_ERROR_MODE_NOT_IMPLEMENTED;
        break;

    default:
        assert(false);
    }
    return data;
}

static void *
_global_panic_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = ALLOCATOR_ERROR_NONE;
    unused(user_ptr);
    void *data = NULL;
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE:
        data = realloc(args.old_ptr, args.new_size);
        assert(data != NULL);
        break;
    case ALLOCATOR_MODE_FREE:
        free(args.old_ptr);
        break;
    case ALLOCATOR_MODE_FREE_ALL:
        *out_error = ALLOCATOR_ERROR_MODE_NOT_IMPLEMENTED;
        break;
    default:
        assert(false);
    }
    return data;
}

static void *
_global_none_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = ALLOCATOR_ERROR_MODE_NOT_IMPLEMENTED;
    unused(user_ptr);
    unused(args);
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE:
    case ALLOCATOR_MODE_FREE:
    case ALLOCATOR_MODE_FREE_ALL:
        break;
    default:
        assert(false);
    }
    return NULL;
}

const Allocator
GLOBAL_HEAP_ALLOCATOR  = {&_global_heap_allocator_fn,  NULL},
GLOBAL_PANIC_ALLOCATOR = {&_global_panic_allocator_fn, NULL},
GLOBAL_NONE_ALLOCATOR  = {&_global_none_allocator_fn,  NULL};

void *
mem_new(Allocator_Error *out_error, size_t size, size_t align, Allocator allocator)
{
    return mem_rawnew(out_error, size, align, allocator);
}

Allocator_Error
mem_free(void *ptr, size_t size, Allocator allocator)
{
    return mem_rawfree(ptr, size, allocator);
}

Allocator_Error
mem_free_all(Allocator allocator)
{
    Allocator_Error error;
    allocator.fn(&error, allocator.user_ptr, ALLOCATOR_MODE_FREE_ALL, (Allocator_Args){0});
    return error;
}

void *
mem_make(Allocator_Error *out_error, size_t count, size_t size, size_t align, Allocator allocator)
{
    return mem_rawnew(out_error, count * size, align, allocator);
}

void *
mem_resize(Allocator_Error *out_error, void *old_ptr, size_t old_count, size_t new_count, size_t size, size_t align, Allocator allocator)
{
    size_t old_size = old_count * size;
    size_t new_size = new_count * size;
    return mem_rawresize(out_error, old_ptr, old_size, new_size, align, allocator);
}

Allocator_Error
mem_delete(void *ptr, size_t count, size_t size, Allocator allocator)
{
    return mem_rawfree(ptr, count * size, allocator);
}

void *
mem_rawnew(Allocator_Error *out_error, size_t size, size_t align, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = NULL,
        .old_size   = 0,
        .new_size   = size,
        .alignment  = align,
    };
    return allocator.fn(out_error, allocator.user_ptr, ALLOCATOR_MODE_ALLOC, args);
}

void *
mem_rawresize(Allocator_Error *out_error, void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = old_ptr,
        .old_size   = old_size,
        .new_size   = new_size,
        .alignment  = align,
    };
    return allocator.fn(out_error, allocator.user_ptr, ALLOCATOR_MODE_RESIZE, args);
}

Allocator_Error
mem_rawfree(void *ptr, size_t size, Allocator allocator)
{
    Allocator_Args args = {
        .old_ptr    = ptr,
        .old_size   = size,
        .new_size   = 0,
        .alignment  = 0,
    };
    Allocator_Error error;
    allocator.fn(&error, allocator.user_ptr, ALLOCATOR_MODE_FREE, args);
    return error;
}

#endif // ALLOCATOR_IMPLEMENTATION

// The macro definitions should come AFTER the implementation so that we don't
// have name collisions during function definitions.
#define mem_new(T, out_error, allocator)                                       \
    cast(T *)mem_new(out_error, sizeof(T), alignof(T), allocator)

#define mem_free(ptr, allocator)                                               \
    mem_free(ptr, sizeof(*(ptr)), allocator)

#define mem_make(T, out_error, count, allocator)                               \
    cast(T *)mem_make(                                                         \
        out_error,                                                             \
        count,                                                                 \
        sizeof(T),                                                             \
        alignof(T),                                                            \
        allocator)                                                             \

#define mem_resize(T, out_error, old_ptr, old_count, new_count, allocator)     \
    cast(T *)mem_resize(                                                       \
        out_error,                                                             \
        old_ptr,                                                               \
        old_count,                                                             \
        new_count,                                                             \
        sizeof(T), alignof(T),                                                 \
        allocator)

#define mem_delete(ptr, count, allocator)                                      \
    mem_delete(ptr, count, sizeof(*(ptr)), allocator)
