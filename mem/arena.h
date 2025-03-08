#pragma once

#include "../common.h"
#include "allocator.h"

#ifndef ARENA_PAGE_SIZE

// Size of the `Memory_Block` header along with its buffer.
#define ARENA_PAGE_SIZE    4096

#endif // ARENA_PAGE_SIZE

typedef struct Memory_Block Memory_Block;
struct Memory_Block {
    Memory_Block *prev; // The previous block, likely filled up.
    size_t        used; // The current number of actively used bytes in `base`.
    size_t        size; // The total number of bytes in `base`.
    char          base[]; // Size is usually in relation to `ARENA_PAGE_SIZE`.
};

static_assert(ARENA_PAGE_SIZE > sizeof(Memory_Block), "ARENA_PAGE_SIZE too small to hold header and data");

typedef struct Arena Arena;
struct Arena {
    Memory_Block *begin; // Primary block we are allocating from.
    Memory_Block *end;   // The oldest block we have.
};

/**
 * @brief
 *      Initializes `arena` with 1 memory block's worth of `ARENA_PAGE_SIZE`.
 *
 * @return
 *      The error upon the allocation of the said memory block.
 *
 * @note
 *      How the memory is allocated is determined by your platform. E.g. on Linux,
 *      we use `mmap()` whereas on Windows we use `VirtualAlloc()`.
 */
Allocator_Error
arena_init(Arena *arena);

/**
 * @brief
 *      Deallocates all the memory, including our owned memory blocks, associated
 *      with `arena`.
 *
 * @note
 *      Take care to not have any dangling pointers lying around after this is
 *      called! Due to our implementation, address sanitizers won't catch them
 *      because all they know is that we own that memory and can thus write to it.
 */
void
arena_destroy(Arena *arena);

/**
 * @brief
 *      Create a stack-allocated `Allocator` instance out of an `Arena *`.
 *      This is useful to provide access to the general `Allocator` interface.
 *
 * @note
 *      Ensure that `arena` is valid for the duration of the resulting allocator!
 *      Otherwise, if at any point `arena` becomes invalid, your allocator will
 *      be completely broken!
 */
Allocator
arena_allocator(Arena *arena);

/**
 * @brief
 *      Low level memory allocation function for `Arena`. Always gives you back
 *      a new and unique pointer.
 *
 * @note
 *      You probably don't want to use this directly unless you are allocating
 *      something like a struct with a flexible-array-member.
 *
 *      You should instead use an arena wrapped in an `Allocator` and use the
 *      helper macros `mem_new` and `mem_make`.
 */
void *
arena_rawalloc(Arena *arena, size_t size, size_t align);

/**
 * @brief
 *      Low level memory resizing function for `Arena`. May give you back the
 *      same pointer if the allocation can be extended. The resulting pointer
 *      will have the old data copied over if applicable.
 *
 * @note
 *      You probably don't want to use this directly. You should instead use an
 *      arena wrapped in an `Allocator` and use the helper macro `mem_resize`.
 */
void *
arena_rawresize(Arena *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align);

/**
 * @brief
 *      Frees all allocated sub-arenas in our list of owned memory blocks, except
 *      for the very first one we allocated.
 *
 * @note
 *      How the freeing of a `Memory_Block` is implemented depends on your platform.
 *      E.g. on Linux we use `munmap()` whereas on Windows we use `VirtualFree()`.
 */
void
arena_free_all(Arena *arena);

/**
 * @brief
 *      Calculate how many bytes are actively allocated across all the memory
 *      blocks owned by `arena`.
 *
 * @param out_total
 *      Optional. Pass the address of a `size_t` if you want to calculate how
 *      many bytes are allocated across all the owned memory blocks. Otherwise,
 *      pass `NULL`.
 *
 * @note
 *      For `out_total`, the written value does not consider the space taken up
 *      by the `Memory_Block` header.
 */
size_t
arena_get_usage(const Arena *arena, size_t *out_total);

#ifdef ARENA_IMPLEMENTATION

#include <assert.h> // assert
#include <string.h> // memcpy

static void *
_arena_allocator_fn(Allocator_Error *out_error, void *user_ptr, Allocator_Mode mode, Allocator_Args args)
{
    Arena *arena = cast(Arena *)user_ptr;
    void  *data  = NULL;
    *out_error = ALLOCATOR_ERROR_NONE;
    switch (mode) {
    case ALLOCATOR_MODE_ALLOC:
        data = arena_rawalloc(arena, args.new_size, args.alignment);
        if (data == NULL)
            *out_error = ALLOCATOR_ERROR_OUT_OF_MEMORY;
        break;

    case ALLOCATOR_MODE_RESIZE:
        data = arena_rawresize(arena, args.old_ptr, args.old_size, args.new_size, args.alignment);
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

#ifdef __unix__

// NOTE: `MAP_ANONYMOUS` is only available with Linux kernel version >= 2.4.
// You may define `_DEFAULT_SOURCE` to get it. Note that because of our header-only
// nature, it is defined in `common.h` and `common.h` must be included before
// any standard library headers.
#include <sys/mman.h>

static inline Memory_Block *
_arena_memory_block_new(size_t size, Memory_Block *prev)
{
    Memory_Block *block = cast(Memory_Block *)mmap(
        /* addr   */ NULL,
        /* len    */ size,
        /* prot   */ PROT_READ | PROT_WRITE,
        /* flags  */ MAP_ANONYMOUS | MAP_PRIVATE,
        /* fd     */ -1,
        /* offset */ 0);

    // Technically mmap *can* map to `NULL`, but it is highly unlikely.
    if (block == MAP_FAILED)
        return NULL;
    block->prev  = prev;
    block->used  = 0;
    block->size  = size - sizeof(*block); // our allocation includes the header
    return block;
}

static void
_arena_memory_block_free(Memory_Block *block)
{
    int result = munmap(block, block->size + sizeof(*block));
    // If *this* fails, something horribly wrong has happened.
    //  -   Either we supplied a bad pointer to `addr` or a bad length to `len`.
    //  -   This assertion saved me from when I subtracted `new_size` from
    //      `block->used` instead of subtracting `old_size` which caused unsigned
    //      integer overflow, because `new_size` was greater than `old_size`!
    assert(result == 0);
}

#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
static inline Memory_Block *
_arena_memory_block_new(size_t size, Memory_Block *prev)
{
    Memory_Block *block = cast(Memory_Block *)VirtualAllocEx(
        /* hProcess         */ GetCurrentProcess(),
        /* lpAddress        */ NULL,
        /* dwSize           */ size,
        /* flAllocationType */ MEM_RESERVE | MEM_COMMIT,
        /* flProtect        */ PAGE_READWRITE);

    if (block != NULL) {
        block->prev  = NULL;
        block->used  = 0;
        block->size  = size - sizeof(*block);
    }
    return block;
}

// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
static void
_arena_memory_block_free(Memory_Block *block)
{
    BOOL ok = VirtualFreeEx(GetCurrentProcess(), block, block->size + sizeof(*block), MEM_RELEASE);
    assert(ok);
}

#else // _WIN32

#warning Using `malloc`. Please add platform-specific functionality if you can!

#include <stdlib.h> // malloc, free

// Malloc always works, although there's no guarantee it's page-aligned.
static inline Memory_Block *
_arena_memory_block_new(size_t size, Memory_Block *prev)
{
    Memory_Block *block = cast(Memory_Block *)malloc(size);
    if (block != NULL) {
        block->prev  = NULL;
        block->used  = 0;
        block->size  = size - sizeof(*block);
    }
    return block;
}

static void
_arena_memory_block_free(Memory_Block *block)
{
    free(block);
}

#endif // __unix__

/**
 * @brief
 *      Type-safe helper function. Certainly beats the `max` macro!
 */
static inline size_t
_arena_max(size_t a, size_t b)
{
    return a > b ? a : b;
}

Allocator_Error
arena_init(Arena *arena)
{
    Memory_Block *block = _arena_memory_block_new(ARENA_PAGE_SIZE, NULL);
    if (block == NULL)
        return ALLOCATOR_ERROR_OUT_OF_MEMORY;
    *arena = (Arena){
        .begin      = block,
        .end        = block,
    };
    return ALLOCATOR_ERROR_NONE;
}

void
arena_destroy(Arena *arena)
{
    for (Memory_Block *block = arena->begin; block != NULL;) {
        Memory_Block *prev = block->prev;
        _arena_memory_block_free(block);
        block = prev;
    }
    arena->begin = NULL;
    arena->end   = NULL;
}

Allocator
arena_allocator(Arena *arena)
{
    Allocator allocator = {.fn = &_arena_allocator_fn, .user_ptr = arena};
    return allocator;
}

/**
 * @brief
 *      Internal implementation function. This makes many assumptions.
 *      Do NOT use!
 *
 * @note
 *      This function assumes the following:
 *          1. `block` has at least `size` bytes available, not accounting for alignment.
 *          2. Because of #1, we don't need to allocate a new block.
 */
static void *
_arena_memory_block_rawalloc(Memory_Block *block, size_t size, size_t align)
{
    uintptr_t base_addr  = cast(uintptr_t)block->base;
    uintptr_t start_addr = base_addr + block->used;

    // Adjust for alignment (assuming alignment is a power of 2)
    // x % pow2 == x & (pow2 - 1)
    while ((start_addr & (align - 1)) != 0) {
        ++start_addr;
    }

    uintptr_t end_addr = start_addr + size;

    // New aligned allocation fits?
    if (end_addr <= base_addr + block->size) {
        // Same concept as getting the length of an array by subtracting 1-past-
        // end-pointer and base pointer.
        block->used = end_addr - base_addr;
        return cast(void *)start_addr;
    }
    return NULL;
}

/**
 * @brief
 *      Internal implementation function. This makes many assumptions.
 *      Do NOT use!
 *
 * @note
 *      This function assumes the following:
 *          1. We absolutely need a new `Memory_Block`.
 *          2. Said memory block can at least fit an allocation of given `size`.
 *          3. We want an allocation from this new block immediately.
 */
static void *
_arena_chain_new_block_and_alloc(Arena *arena, size_t size, size_t align)
{
    // TODO: Ensure this is a power of 2?
    const size_t  block_size = _arena_max(size + sizeof(Memory_Block), ARENA_PAGE_SIZE);
    Memory_Block *new_block  = _arena_memory_block_new(block_size, arena->begin);
    if (new_block == NULL)
        return NULL;

    // `new_block` is now the primary block we'll be allocating from here on.
    arena->begin = new_block;
    return _arena_memory_block_rawalloc(new_block, size, align);
}

void *
arena_rawalloc(Arena *arena, size_t size, size_t align)
{
    // Find the first sub-block that can accomodate our allocation.
    for (Memory_Block *block = arena->begin; block != NULL; block = block->prev) {
        // Do we even have a chance of accomodating this allocation?
        if (block->used + size > block->size)
            continue;

        void *data = _arena_memory_block_rawalloc(block, size, align);
        if (data != NULL)
            return data;
    }

    // We couldn't find a spare block that could accomodate us.
    return _arena_chain_new_block_and_alloc(arena, size, align);
}

void *
arena_rawresize(Arena *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align)
{
    // Try to find the sub-block where `old_ptr` came from. There is no other
    // portable way of doing this!
    for (Memory_Block *block = arena->begin; block != NULL; block = block->prev) {
        // If `old_ptr` matches the most recent allocation, we might be able to
        // extend it.
        const void *latest_alloc = block->base + (block->used - old_size);
        if (old_ptr != latest_alloc)
            continue;

        // If shrinking, just mark the excess memory as reusable.
        bool is_shrink = old_size >= new_size;
        if (is_shrink) {
            block->used -= old_size - new_size;
            return old_ptr;
        }

        // Does extending the allocation fit?
        size_t added_size  = new_size - old_size;
        size_t result_size = block->used + added_size;

        // TODO: check if this is an off-by-one error, or if it's valid
        if (result_size <= block->size) {
            block->used = result_size;
            return old_ptr;
        }

        // We can't extend, so we know we need to get rid of this allocation from
        // this block. We `break` because in `arena_rawalloc()` we will check for
        // blocks that can accomodate us, or allocate a new block.
        block->used -= old_size;
        break;
    }

    // Unable to extend; we should try to get a new allocation from some other
    // block or even allocate a new block that can accomodate us.
    void *new_ptr = arena_rawalloc(arena, new_size, align);

    // memcpy with NULL argument/s is undefined behavior.
    if (new_ptr != NULL && old_ptr != NULL)
        return memcpy(new_ptr, old_ptr, old_size);
    else
        return new_ptr;
}

void
arena_free_all(Arena *arena)
{
    Memory_Block *end = arena->end;
    // Maybe we shouldn't free the extra pages just yet?
    for (Memory_Block *block = arena->begin; block != end;) {
        Memory_Block *prev = block->prev;
        _arena_memory_block_free(block);
        block = prev;
    }
    end->used = 0;
    arena->begin = end;
}

size_t
arena_get_usage(const Arena *arena, size_t *out_total)
{
    size_t used  = 0;
    size_t total = 0;

    for (const Memory_Block *block = arena->begin; block != NULL; block = block->prev) {
        used  += block->used;
        total += block->size;
    }

    if (out_total != NULL)
        *out_total = total;
    return used;
}

#endif // ARENA_IMPLEMENTATION
