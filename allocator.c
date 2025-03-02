#include <stdlib.h>
#include <assert.h>

#include "allocator.h"

#undef mem_new
#undef mem_free
#undef mem_make
#undef mem_resize
#undef mem_delete

static void *
heap_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    unused(user_ptr);
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
    case ALLOCATOR_MODE_RESIZE: {
        void *new_ptr = realloc(args.old_ptr, args.new_size);
        // If `realloc` fails, `old_ptr` is not freed.
        if (new_ptr == NULL)
            free(args.old_ptr);
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
panic_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
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
nil_allocator_fn(void *user_ptr, Allocator_Mode mode, Allocator_Args args)
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
HEAP_ALLOCATOR  = {&heap_allocator_fn, NULL},
PANIC_ALLOCATOR = {&panic_allocator_fn, NULL},
NIL_ALLOCATOR   = {&nil_allocator_fn, NULL};

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
