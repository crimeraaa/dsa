#ifndef TERRIBLE_GENERIC_ARRAY_H
#define TERRIBLE_GENERIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <limits.h>     /* SCHAR_MIN */
#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* size_t, malloc, free */
#include <stdio.h>      /* perror */
#include <wchar.h>      /* wchar_t */
    
#include "typeinfo.h"

typedef unsigned char ga_byte;

typedef struct ga_array {
    const ti_typeinfo *info;
    ga_byte *rawbytes; // Treat our array as just a giant byte-block.
    size_t count;      // Item count (not bytes!) written in the buffer.  
    size_t capacity;   // Total item count (not bytes!) we've allocated for.
} ga_array;

ga_array ga_init(size_t count, const ti_typeinfo *info);
void ga_deinit(ga_array *self);
void *ga_assign(ga_array *self, void *dst, const void *src);
bool ga_push_back(ga_array *self, const void *src);
void *ga_retrieve(const ga_array *self, size_t index);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_ARRAY_H */
