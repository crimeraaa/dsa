#ifndef INTERN_H
#define INTERN_H

#include <stdint.h>

#include "common.h"
#include "strings.h"
#include "allocator.h"

typedef struct {
    size_t   len;
    uint32_t hash;
    char     data[];
} *Intern_String;

typedef struct {
    String        key;
    Intern_String value;
} Intern_Entry;

typedef struct {
    Allocator     allocator;
    Intern_Entry *entries;
    size_t        count;
    size_t        cap;
} Intern;

Intern
intern_make(Allocator allocator);

void
intern_destroy(Intern *intern);

/**
 * @note
 *      If `string` is not yet interned, we will intern it.
 * 
 * @return
 *      A `String` instance which points to the interned string.
 *      This string is valid as long as the map lives.
 */
String
intern_get(Intern *intern, String string);

#endif // INTERN_H
