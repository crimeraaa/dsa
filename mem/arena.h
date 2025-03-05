#pragma once

#include "../common.h"
#include "allocator.h"

#define ARENA_SIZE  4096

typedef struct Memory_Block Memory_Block;
struct Memory_Block {
    Memory_Block *prev;
    size_t        used; // The current number of actively used bytes in `base`.
    size_t        size; // The total number of bytes in `base`.
    char          base[];
};

typedef struct Arena Arena;
struct Arena {
    Memory_Block *memory;
    size_t        total_used; // TODO: Manage
    size_t        total_size; // TODO: Manage
};

Arena
arena_make(void);

void
arena_destroy(Arena *arena);

Allocator
arena_allocator(Arena *arena);

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

Arena
arena_make(void)
{
    Memory_Block *memory = malloc(ARENA_SIZE);
    Arena arena = {
        .memory     = memory,
        .total_size = ARENA_SIZE - sizeof(*memory),
        .total_used = 0,
    };
    assert(memory != NULL);
    memory->prev  = NULL;
    memory->used  = 0;
    memory->size  = ARENA_SIZE - sizeof(*memory);
    return arena;
}

void
arena_destroy(Arena *arena)
{
    Memory_Block *memory = arena->memory;
    while (memory != NULL) {
        Memory_Block *prev = memory->prev;
        free(memory);
        memory = prev;
    }
    arena->memory     = NULL;
    arena->total_used = 0;
    arena->total_size = 0;
}

Allocator
arena_allocator(Arena *arena)
{
    Allocator allocator = {.fn = &_arena_allocator_fn, .user_ptr = arena};
    return allocator;
}

void *
arena_alloc(Arena *arena, size_t size, size_t align)
{
    uintptr_t base_addr  = cast(uintptr_t)arena->memory->base;
    uintptr_t start_addr = base_addr + arena->memory->used;
    // Adjust for alignment (assuming alignment is a power of 2)
    while ((start_addr & (align - 1)) != 0) {
        ++start_addr;
    }
    
    // New aligned allocation fits?
    uintptr_t end_addr = start_addr + size;
    if (end_addr < base_addr + arena->memory->size) {
        arena->memory->used = end_addr - base_addr;
        return cast(void *)start_addr;
    }
    return NULL;
}

void *
arena_resize(Arena *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align)
{
    // Check if `old_ptr` matches the most recent allocation
    void *last_allocation = arena->memory->base + (arena->memory->used - old_size);
    // We might be able to extend the most recent allocation?
    if (old_ptr == last_allocation) {
        // If shrinking, just mark the excess memory as reusable.
        bool is_shrink = old_size >= new_size;
        if (is_shrink) {
            arena->memory->used -= old_size - new_size;
            return old_ptr;
        }
        size_t result_size = arena->memory->used + new_size - old_size;
        if (result_size < arena->memory->size) {
            arena->memory->used = result_size;
            return old_ptr;
        }
        // Can't extend!
        // TODO: Allocate a new sub-arena?
        return NULL;
    }
    // Unable to extend the allocation; allocate a new block.
    return arena_alloc(arena, new_size, align);
}

void
arena_free_all(Arena *arena)
{
    // Free all blocks except the first one.
    Memory_Block *prev = arena->memory->prev;
    while (prev != NULL) {
        Memory_Block *tmp = prev->prev;
        free(prev);
        prev = tmp;
    }
    arena->memory->used = 0;
}

#endif // ARENA_IMPLEMENTATION
