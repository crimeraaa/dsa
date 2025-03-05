#pragma once

#include "../common.h"
#include "allocator.h"

#define ARENA_SIZE  4096

typedef struct Arena Arena;
struct Arena {
    Arena *next;  // Maybe we can be a growing arena?
    size_t used;  // Currently used size, in bytes, of `memory`.
    size_t total; // Total size, in bytes, of `memory`.
    char   memory[];
};

Arena *
arena_make(void);

void
arena_destroy(Arena *arena);

Allocator
arena_to_allocator(Arena *arena);

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
    // TODO: check if we can simply extend the last allocation
    case ALLOCATOR_MODE_RESIZE: {
        size_t new_used = arena->used + args.new_size;
        // Adjust for alignment
        // TODO: could be made faster since alignment is usually a power of 2?
        while ((new_used % args.alignment) != 0) {
            ++new_used;
        }

        // New aligned allocation fits?
        if (new_used < arena->total) 
            data = &arena->memory[arena->used += args.new_size];
        else
            *out_error = ALLOCATOR_ERROR_OUT_OF_MEMORY;
        break;
    }
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
    arena->next  = NULL;
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

void
arena_free_all(Arena *arena)
{
    arena->used = 0;
}

#endif // ARENA_IMPLEMENTATION
