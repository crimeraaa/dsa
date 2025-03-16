#pragma once

#ifdef DSA_IMPLEMENTATION
#define DSA_ALLOCATOR_IMPLEMENTATION
#endif // DSA_IMPLEMENTATION

#include "../common.h"

typedef enum {
    Allocator_Mode_Alloc,
    Allocator_Mode_Resize,
    Allocator_Mode_Free,
    Allocator_Mode_Free_All,
} Allocator_Mode;

typedef struct {
    void  *old_ptr;
    size_t old_size;
    size_t new_size;
    size_t alignment;
} Allocator_Args;

// The zero-value `Allocator_Error_None` indicates success while nonzero indicates
// some failure. You can simply check `if (error)`.
typedef enum {
    Allocator_Error_None,          // Indicates success.
    Allocator_Error_Out_Of_Memory, // Can't fulfill an allocation request.
    Allocator_Error_Mode_Not_Implemented, // e.g. calling `free_all` on the global heap allocator.
} Allocator_Error;

/**
 * @brief
 *      The `Allocator` provides a uniform interface for memory allocation.
 *      In essence, it simply wraps the functionality of an existing allocator.
 *
 *      Please see the documentation for the `mem_*` functions/macros to see
 *      what guarantees must be made by any allocator that chooses to implement
 *      this interface.
 *
 *      Useful examples are `global_heap_allocator` for simply wrappers and
 *      `Arena` from `mem/arena.h` for more involved wrappers.
 */
typedef struct {
    /**
     * @param out_error
     *      The address of an `Allocator_Error` variable. Must be non-null.
     *      The reason we do it like this is so that we can wrap the returned
     *      pointer with a cast.
     *
     *      If we instead took an out-parameter of type `void **` we would need
     *      to cast the *address* of a pointer to `void **`, e.g.
     *      `mem_rawresize(char, (void **)&buffer, ...)`.
     *
     *      I don't know why, but casting the *address* of something already feels
     *      off. For me, casting to and from 1 level of indirection is already enough.
     */
    void *(*fn)(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args);

    /**
     * @brief
     *      In good old C fashion, this is what is known as the 'context' or
     *      'userdata' pointer. This is simply the address of extra information
     *      that is known at the time of creation for a particular `Allocator`
     *      instance and passed to its respective callback function `fn`.
     *
     *      For a good example, see the `Arena` implementation in `mem/arena.h`.
     */
    void *user_ptr;
} Allocator;

// A simple wrapper around the `malloc` family.
extern const Allocator global_heap_allocator;

// Panics when an allocation request cannot be fulfilled.
extern const Allocator global_panic_allocator;

// An interface that simply returns `NULL` for all allocation requests.
// This is useful for types that require an allocator interface but can use
// fixed-size memory, e.g. `String_Builder`.
extern const Allocator global_none_allocator;

/**
 * @brief
 *      Low-level memory allocation function for the `Allocator` interface.
 *
 *      The given `allocator` must ensure that the resulting pointer is new and
 *      unique from any non-freed ones.
 *
 * @note
 *      You probably don't want to use this function directly.
 *      You should instead use the helper macros `mem_new` and `mem_make`.
 *      Please see their documentation for more information.
 */
void *
mem_rawnew(Allocator_Error *out_error, size_t size, size_t align, Allocator allocator);

/**
 * @brief
 *      Low-level memory reallocation function for the `Allocator` interface.
 *
 *      Unlike `mem_rawnew`, and thus the helper macros `mem_new` and `mem_make`,
 *      this function may not return a unique pointer every time. Some allocators
 *      may be able to 'extend' the allocation pointed to by `old_ptr`.
 *
 *      Also, if `old_size` is actually greater than `new_size`, it is possible
 *      for the allocator to simply return `old_ptr` immediately with or without
 *      changing the bookkeeping data.
 *
 * @note
 *      In the event that a new pointer is allocated, the old data will be copied
 *      over. Your allocator may also choose to free `old_ptr`. However, this is
 *      not a strict requirement.
 *
 *      However, if allocation fails, then `old_ptr` must NOT be freed no matter
 *      what. This is because the caller may wish to handle that error somehow.
 *
 * @note
 *      You probably don't want to use this function directly.
 *      You should instead use the helper macro `mem_resize`.
 *      Please see its documentation for more information.
 */
void *
mem_rawresize(Allocator_Error *out_error, void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator);


/**
 * @brief
 *      Low-level memory deallocation function for the `Allocator` interface.
 *
 *      This function simply tells `allocator` that the memory pointed to by `ptr`
 *      (and the `size` bytes following it) are available to be re-used. How the
 *      allocator goes about this is implementation-dependent. Some allocators may
 *      not even implement a freeing function!
 *
 * @note
 *      You probably don't want to use this function directly.
 *      You should instead use the helper macro `mem_free` or `mem_delete`.
 *      Please see their documentation for more information.
 */
Allocator_Error
mem_rawfree(void *ptr, size_t size, Allocator allocator);

/**
 * @brief
 *      Low-level memory deallocation function for the `Allocator` interfaces
 *      for allocators which support freeing all their owned memory at once,
 *      such as arena allocators.
 *
 * @note
 *      For allocators that do not support this operation (e.g. heap allocators)
 *      simple make the callback function write `Allocator_Error_Mode_Not_Implemented`
 *      to the `Allocator_Error` out-parameter.
 */
Allocator_Error
mem_free_all(Allocator allocator);

/**
 * @brief
 *      Inspired by the Odin programming language. Allocates enough memory
 *      for a single instance of type `T`.
 *
 * @param out_error
 *      The address of an `Allocator_Error` variable. e.g.
 *
 *  ```c
 *          Allocator_Error error;
 *          int *refcount = mem_new(int, &error, allocator);
 *          if (error) { ... } // handle errors here
 *  ```
 *
 *      See the documentation for the `fn` member in `Allocator` for more
 *      information.
 *
 * @note
 *      To free memory resulting from this function, use `mem_new`.
 *      Do NOT use `mem_delete` as you might accidentally specify the wrong
 *      number of elements.
 */
#define mem_new(T, out_error, allocator)                                       \
    cast(T *)mem_rawnew(                                                       \
        out_error,                                                             \
        sizeof(T),                                                             \
        alignof(T),                                                            \
        allocator)

/**
 * @brief
 *      Inspired by the Odin programming language. Deallocates the memory that
 *      `ptr` points to, assuming it only points to 1 instance of the type
 *      pointed to by `ptr`.
 *
 * @note
 *      You should only call this function with memory you got from `mem_new`.
 *      If you pass in memory from `mem_make`, your allocator may not have the
 *      book-keeping data to know how many elements are being pointed to!
 *
 * @return
 *      An `Allocator_Error`. If successful, it will be `Allocator_Error_None`,
 *      which is the zero value. You may thus check for failure with the idiom
 *      `if (error)`, e.g.
 *
 * ```c
 *      int *refcount = ...; // Allocate, check for errors, then use
 *      Allocator_Error error_free = mem_free(refcount, allocator);
 *      if (error_free) { ... } // What happened here?
 * ```
 */
#define mem_free(ptr, allocator)                                               \
    mem_rawfree(                                                               \
        ptr,                                                                   \
        sizeof(*(ptr)),                                                        \
        allocator)

/**
 * @brief
 *      Inspired by the Odin programming language. Allocates enough memory for
 *      `count` instances of type `T`.
 *
 * @param out_error
 *      The address of an `Allocator_Error`. See the documentation for the
 *      `fn` member in `Allocator` for more information.
 *
 * @note
 *      Free the memory received from this function using the `mem_delete` macro.
 *      Do NOT use `mem_free`, as your allocator may not have the book-keeping
 *      data to know how many elements are being pointed to!
 */
#define mem_make(T, out_error, count, allocator)                               \
    cast(T *)mem_rawnew(                                                       \
        out_error,                                                             \
        sizeof(T) * (count),                                                   \
        alignof(T),                                                            \
        allocator)

/**
 * @brief
 *      Inspired by the Odin programming language. Reallocates the memory
 *      for `old_ptr`. May return the same pointer or allocate a new one with
 *      the old data copied over.
 *
 * @note
 *      If this succeeds (that is, the result is non-`NULL`) then the memory
 *      pointed to by `old_ptr` may be freed by the allocator.
 *
 *      However, if it fails (the result is `NULL`), then `old_ptr` is guaranteed
 *      to not be freed. What you do in that situation is up to you.
 */
#define mem_resize(T, out_error, old_ptr, old_count, new_count, allocator)     \
    cast(T *)mem_rawresize(                                                    \
        out_error,                                                             \
        old_ptr,                                                               \
        sizeof(T) * (old_count),                                               \
        sizeof(T) * (new_count),                                               \
        alignof(T),                                                            \
        allocator)

/**
 * @brief
 *      Inspired by the Odin programming language. Deallocates memory pointed to
 *      by `ptr`, assuming it points to `count` elements of type specified by
 *      `ptr`.
 */
#define mem_delete(ptr, count, allocator)                                      \
    mem_rawfree(                                                               \
        ptr,                                                                   \
        sizeof(*ptr) * count,                                                  \
        allocator)

#ifdef DSA_ALLOCATOR_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>

//=== GLOBAL ALLOCATOR WRAPPERS ============================================ {{{

static void *
_global_heap_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = Allocator_Error_None;
    unused(user_ptr);
    void *data = NULL;
    switch (mode) {
    case Allocator_Mode_Alloc:
    case Allocator_Mode_Resize:
        // NOTE: If `realloc` fails, `old_ptr` is not freed.
        // Don't free it here, because the caller might still need it!
        data = realloc(args.old_ptr, args.new_size);
        if (data == NULL)
            *out_error = Allocator_Error_Out_Of_Memory;
        break;

    case Allocator_Mode_Free:
        free(args.old_ptr);
        break;

    case Allocator_Mode_Free_All:
        *out_error = Allocator_Error_Mode_Not_Implemented;
        break;

    default:
        assert(false);
    }
    return data;
}

static void *
_global_panic_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = Allocator_Error_None;
    unused(user_ptr);
    void *data = NULL;
    switch (mode) {
    case Allocator_Mode_Alloc:
    case Allocator_Mode_Resize:
        data = realloc(args.old_ptr, args.new_size);
        assert(data != NULL);
        break;
    case Allocator_Mode_Free:
        free(args.old_ptr);
        break;
    case Allocator_Mode_Free_All:
        *out_error = Allocator_Error_Mode_Not_Implemented;
        break;
    default:
        assert(false);
    }
    return data;
}

static void *
_global_none_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    *out_error = Allocator_Error_Mode_Not_Implemented;
    unused(user_ptr);
    unused(args);
    switch (mode) {
    case Allocator_Mode_Alloc:
    case Allocator_Mode_Resize:
    case Allocator_Mode_Free:
    case Allocator_Mode_Free_All:
        break;
    default:
        assert(false);
    }
    return NULL;
}

const Allocator
global_heap_allocator  = {&_global_heap_allocator_fn,  NULL},
global_panic_allocator = {&_global_panic_allocator_fn, NULL},
global_none_allocator  = {&_global_none_allocator_fn,  NULL};

//=== }}} ======================================================================

Allocator_Error
mem_free_all(Allocator allocator)
{
    Allocator_Error error;
    allocator.fn(&error, allocator.user_ptr, Allocator_Mode_Free_All, (Allocator_Args){0});
    return error;
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
    return allocator.fn(out_error, allocator.user_ptr, Allocator_Mode_Alloc, args);
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
    return allocator.fn(out_error, allocator.user_ptr, Allocator_Mode_Resize, args);
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
    allocator.fn(&error, allocator.user_ptr, Allocator_Mode_Free, args);
    return error;
}

#endif // DSA_ALLOCATOR_IMPLEMENTATION
