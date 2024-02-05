#ifndef TERRIBLE_GENERIC_ARRAY_H
#define TERRIBLE_GENERIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* size_t, malloc, free */
    
#include "typeinfo.h"

typedef unsigned char ga_byte;

typedef struct ga_array {
    const ti_typeinfo *info;
    ga_byte *rawbytes; // Treat our array as just a giant byte-block.
    size_t count;      // Item count (not bytes!) written in the buffer.  
    size_t capacity;   // Total item count (not bytes!) we've allocated for.
} ga_array;

/**
 * @brief   Start up a new instance of `ga_array`!
 * 
 * @param   count   How many elements should the array hold at first?
 * @param   info    What information/functions do we need to know about this type?
 * 
 * @note    Most fundamental types are already accounted for and have their own
 *          pre-defined `ti_typeinfo`. See the `ti_query` function and its usage.
 * @note    ---
 * @note    For non-fundamental types (e.g. most `struct` definitions), 
 *          you'll have to create an instance of `ti_typeinfo`.
 *          See `typeinfo.h` and `typeinfo.c` on how you can do that!
 *          
 * @return  Stack-allocated `ga_array` struct instance.
 */
ga_array ga_init(size_t count, const ti_typeinfo *info);

/**
 * @brief   Calls the associated `ti_deinitfn` on all elements, then frees
 *          the internal array buffer.
 * 
 * @param   self    Address of your `ga_array` instance.
 * 
 * @note    It is very important to set up `ti_typeinfo` for this to work!
 */
void ga_deinit(ga_array *self);

/**
 * @brief   Calls the associated `ti_copyfn`, or `self->info->fnlist->copy`.
 *          It receives `dst` and `src` as its arguments.
 *          
 * @param   self    Address of your `ga_array` instance.
 * @param   dst     Address of the variable to have data stored in.
 * @param   src     Address of the data to copy from.
 * 
 * @return  Return value of `copy`. You can define the copy function in such a
 *          way that it returns `nullptr` on certain conditions.
 */
void *ga_assign(ga_array *self, void *dst, const void *src);

/**
 * @brief   Resizes the internal buffer via `realloc`, which also takes care
 *          of copying/moving over data.
 *          
 * @return  `true` if successful, else `false` if something went wrong.
  */
bool ga_resize(ga_array *self, size_t newcapacity);

/**
 * @brief   Try to append the value pointed to by `src` to an element located
 *          at the current `self->count` index.
 *          
 * @return  `true` if all went well else `false` if resizing buffer failed.
 */
bool ga_push_back(ga_array *self, const void *src);

/**
 * @brief   Get a handle to the `index`'th element in our array.
 *          Performs a bounds check as well.
 * 
 * @return  Handle to the element or `nullptr` if `index` was out of bounds.
 * 
 * @warning No const correctness is guaranteed for the return value!
 */
void *ga_retrieve(const ga_array *self, size_t index);

/**
 * @brief   Similar to `ga_retrieve` but without the bounds check.
 *          Use at your own risk!
 */
void *ga_rawretrieve(const ga_array *self, size_t index);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_ARRAY_H */
