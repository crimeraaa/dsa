#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdalign.h>

#include "common.h"

typedef enum {
    ALLOCATOR_MALLOC,
    ALLOCATOR_MRESIZE,
    ALLOCATOR_MFREE,
} Allocator_Mode;

typedef void *(*Allocator_Fn)(void *user_ptr, Allocator_Mode mode, void *old_ptr, size_t old_size, size_t new_size, size_t align);

typedef struct {
    Allocator_Fn fn;
    void *user_ptr;
} Allocator;

extern const Allocator
HEAP_ALLOCATOR,
NIL_ALLOCATOR;


//=== SINGLE OBJECT MEMORY FUNCTIONS ======================================= {{{

void
mem_free(void *ptr, size_t size, Allocator allocator);

__attribute__((
    __malloc__ (mem_free),
    __warn_unused_result__))
void *
mem_new(size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== ARRAY MEMORY FUNCTIONS =============================================== {{{

__attribute__((__copy__ (mem_free)))
void
mem_delete(void *ptr, size_t count, size_t size, Allocator allocator);

__attribute__((
    __malloc__ (mem_delete),
    __warn_unused_result__))
void *
mem_make(size_t count, size_t size, size_t align, Allocator allocator);

__attribute__((
    __malloc__ (mem_delete, 1),
    __warn_unused_result__))
void *
mem_resize(void *old_ptr, size_t old_count, size_t new_count, size_t size, size_t align, Allocator allocator);

//=== }}} ======================================================================

//=== RAW MEMORY FUNCTIONS === ============================================= {{{

__attribute__((__copy__ (mem_delete)))
void
mem_rawfree(void *ptr, size_t size, Allocator allocator);

__attribute__((__warn_unused_result__))
void *
mem_rawnew(size_t size, size_t align, Allocator allocator);

__attribute__((__copy__ (mem_rawnew)))
void *
mem_rawresize(void *old_ptr, size_t old_size, size_t new_size, size_t align, Allocator allocator);

//=== }}} ======================================================================

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

#endif // ALLOCATOR_H
