#ifndef INTERN_H
#define INTERN_H

#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "strings.h"
#include "allocator.h"

// Opaque type so you don't get any funny ideas!
typedef struct Intern_Entry Intern_Entry;

typedef struct {
    Allocator     allocator;
    Intern_Entry *entries;
    size_t        count;
    size_t        cap; // Must always be a power of 2.
    int           max_probe;
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

const char *
intern_get_cstring(Intern *intern, String text);

void
intern_print(const Intern *intern, FILE *stream);

#endif // INTERN_H
