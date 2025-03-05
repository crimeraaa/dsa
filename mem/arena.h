#pragma once

#include "../common.h"
#include "allocator.h"

#define ARENA_SIZE  4096

typedef struct Arena Arena;
struct Arena {
    size_t used;  // Currently used size, in bytes, of `memory`.
    size_t total; // Total size, in bytes, of `memory`.
    char   memory[]; // We assume this (the base memory address) is properly aligned.
};

Arena *
arena_make(void);

void
arena_destroy(Arena *arena);

Allocator
arena_to_allocator(Arena *arena);

void *
arena_alloc(Arena *arena, size_t size, size_t align);

void *
arena_resize(Arena *arena, void *oldtr, size_t oldsize, size_t newsize, size_t align);

void
arena_free_all(Arena *arena);

#ifdef ARENA_IMPLEMENTATION

#include <assert.h>
#include <stdlib.h>

static void *
_arena_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    Arena *arena = cast(Arena *)user_ptr;
    void  *data  = NULL;
    *out_error = ALLOCATOR_ERROR_NONE;
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
        data = arena_alloc(arena, args.new_size, args.alignment);
        if (data == NULL)
            *out_error = ALLOCATOR_ERROR_OUT_OF_MEMORY;
        break;

    case ALLOCATOR_MODE_RESIZE:
        data = arena_resize(arena, args.old_ptr, args.old_size, args.new_size, args.alignment);
        if (data == NULL)
            *out_error = ALLOCATOR_ERROR_OUT_OF_MEMORY;
        break;

    case ALLOCATOR_MODE_FREE:
        *out_error = ALLOCATOR_ERROR_MODE_NOT_IMPLEMENTED;
        break;

    case ALLOCATOR_MODE_FREE_ALL:
        arena_free_all(arena);
        break;

    default:
        assert(false);
    }
    return data;
}

Arena *
arena_make(void)
{
    Arena *arena = malloc(sizeof(*arena) + ARENA_SIZE);
    if (arena == NULL)
        assert(false && "Failed to allocate an arena");
    arena->used  = 0;
    arena->total = ARENA_SIZE;
    return arena;
}

void
arena_destroy(Arena *arena)
{
    free(arena);
}

Allocator
arena_to_allocator(Arena *arena)
{
    Allocator allocator = {.fn = &_arena_allocator_fn, .user_ptr = arena};
    return allocator;
}

void *
arena_alloc(Arena *arena, size_t size, size_t align)
{
    size_t start = arena->used;
    // Adjust for alignment (assuming alignment is a power of 2)
    while ((start & (align - 1)) != 0) {
        ++start;
    }

    // New aligned allocation fits?
    if (start + size < arena->total)  {
        arena->used = start + size;
        return &arena->memory[start];
    }
    return NULL;
}

void *
arena_resize(Arena *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align)
{
    // Check if `old_ptr` matches the most recent allocation
    void *last_allocation = arena->memory + (arena->used - old_size);
    // We might be able to extend the most recent allocation?
    if (old_ptr == last_allocation) {
        // Shrinking; just mark the now-freed region as able to be reused
        if (old_size >= new_size) {
            arena->used -= old_size - new_size;
            return old_ptr;
        }
        size_t added_size = new_size - old_size;
        if (arena->used + added_size < arena->total) {
            arena->used += added_size;
            return old_ptr;
        }
    }
    // Unable to extend the allocation; allocate a new block.
    return arena_alloc(arena, new_size, align);
}

void
arena_free_all(Arena *arena)
{
    arena->used = 0;
}

#endif // ARENA_IMPLEMENTATION
