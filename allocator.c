#include <stdlib.h>
#include <assert.h>

#include "allocator.h"

#undef mem_new
#undef mem_free
#undef mem_make
#undef mem_resize
#undef mem_delete

static void *
heap_allocator_fn(void *user_ptr, Allocator_Mode mode, void *old_ptr, size_t old_size, size_t new_size, size_t align)
{
    unused(user_ptr);
    unused(old_size);
    unused(align);
    switch (mode) {
    case ALLOCATOR_MALLOC:
    case ALLOCATOR_MRESIZE: {
        void *new_ptr = realloc(old_ptr, new_size);
        assert(new_ptr != NULL);
        return new_ptr;
    }
    case ALLOCATOR_MFREE: {
        free(old_ptr);
        return NULL;
    }
    default:
        __builtin_unreachable();
    }
}

static void *
nil_allocator_fn(void *user_ptr, Allocator_Mode mode, void *old_ptr, size_t old_size, size_t new_size, size_t align)
{
    unused(user_ptr);
    unused(old_ptr);
    unused(old_size);
    unused(new_size);
    unused(align);

    switch (mode) {
    case ALLOCATOR_MALLOC:
    case ALLOCATOR_MRESIZE:
    case ALLOCATOR_MFREE:
        break;
    default:
        __builtin_unreachable();
    }
    return NULL;
}

const Allocator
HEAP_ALLOCATOR = {&heap_allocator_fn, NULL},
NIL_ALLOCATOR  = {&nil_allocator_fn, NULL};

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
    return allocator.fn(allocator.user_ptr, ALLOCATOR_MALLOC, NULL, 0, size, align);
}

void *
mem_rawresize(void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator)
{
    return allocator.fn(allocator.user_ptr, ALLOCATOR_MRESIZE, old_ptr, old_size, new_size, align);
}

void
mem_rawfree(void *ptr, size_t size, Allocator allocator)
{
    allocator.fn(allocator.user_ptr, ALLOCATOR_MFREE, ptr, size, 0, 0);
}
