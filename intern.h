#pragma once

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

typedef struct {
    size_t   len;
    uint32_t hash;
    char     data[];
} Intern_String;

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

/**
 * @note
 *      Same semantics as `intern_get` and `intern_get_cstring`.
 *      If `text` is not already interned, we will intern it.
 *
 * @note
 *      Pointers are directly comparable. That is:
 *      `intern_get_interned("Hello") == intern_get_interned("Hello")`.
 */
const Intern_String *
intern_get_interned(Intern *intern, String text);

#ifdef INTERN_IMPLEMENTATION

#include <string.h> // memcmp, memcpy (likely highly optimized)
#include <stdio.h>  // fprintf

struct Intern_Entry {
    Intern_String *value;
    int            probe; // Our distance from our ideal position `hash % cap`.
};

Intern
intern_make(Allocator allocator)
{
    Intern intern = {
        .allocator = allocator,
        .entries   = NULL,
        .count     = 0,
        .cap       = 0,
        .max_probe = 0,
    };
    return intern;
}

void
intern_destroy(Intern *intern)
{
    Allocator     allocator = intern->allocator;
    Intern_Entry *entries   = intern->entries;
    const size_t  cap       = intern->cap;

    for (size_t i = 0; i < cap; ++i) {
        Intern_String *value = entries[i].value;
        if (value == NULL)
            continue;

        mem_rawfree(value, sizeof(*value) + (value->len + 1), allocator);
    }
    mem_delete(entries, cap, allocator);
    intern->entries = NULL;
    intern->count   = 0;
    intern->cap     = 0;
}

#define FNV_OFFSET  2166136261
#define FNV_PRIME   16777619

static uint32_t
_intern_hash(String data)
{
    uint32_t hash = FNV_OFFSET;
    string_for_each(byte, data) {
        // Can't cast the expression to `uint32_t`? Is this not defined behavior?
        hash ^= cast(unsigned char)byte;
        hash *= FNV_PRIME;
    }
    return hash;
}

// Clean up global namespace since we are header-only now
#undef FNV_OFFSET
#undef FNV_PRIME

// We pass `entries` directly in the case of `_intern_resize()`.
static Intern_Entry *
_intern_get(Intern_Entry entries[], size_t cap, String string, uint32_t hash, int *probe)
{
    // Division (and by extension, modulo) by zero is undefined behavior.
    if (cap == 0)
        return NULL;

    // Micro-optimization to avoid constant pointer dereferences.
    int _probe = 0;
    for (size_t i = cast(size_t)hash % cap; /* empty */; ++_probe, i = (i + 1) % cap) {
        Intern_String *istring = entries[i].value;

        // This string isn't interned yet.
        if (istring == NULL)
            goto set_result;

        // Probably not a match. Keep looking.
        if (istring->hash != hash || istring->len != string.len)
            continue;

        // Confirm if it is indeed a match.
        if (memcmp(string.data, istring->data, istring->len) == 0) set_result: {
            *probe = _probe;
            return &entries[i];
        }
    }
    __builtin_unreachable();
}

static void
_intern_update_max_probe(Intern *intern, int probe)
{
    if (probe > intern->max_probe)
        intern->max_probe = probe;
}

static bool
_intern_resize(Intern *intern, size_t new_cap)
{
    Allocator     allocator   = intern->allocator;
    Intern_Entry *new_entries = mem_make(Intern_Entry, new_cap, allocator);
    if (new_entries == NULL)
        return false;

    // Zero out the new memory so that we can safely read them.
    memset(new_entries, 0, sizeof(new_entries[0]) * new_cap);

    // Copy the old non-empty entries.
    size_t        new_count   = 0;
    Intern_Entry *old_entries = intern->entries;
    size_t        old_cap     = intern->cap;

    intern->max_probe = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        // Ignore unset keys; these are yet to be interned.
        if (old_entries[i].value == NULL)
            continue;

        int            probe;
        Intern_String *interned  = old_entries[i].value;
        String         key       = {interned->data, interned->len};
        Intern_Entry  *new_entry = _intern_get(new_entries, new_cap, key, interned->hash, &probe);

        // Probe may be different now that we've resized the hash table.
        *new_entry       = old_entries[i];
        new_entry->probe = probe;
        ++new_count;

        _intern_update_max_probe(intern, probe);
    }

    mem_delete(old_entries, old_cap, allocator);
    intern->entries = new_entries;
    intern->count   = new_count;
    intern->cap     = new_cap;
    return true;
}

static void
_intern_swap_entries(Intern_Entry *restrict a, Intern_Entry *restrict b)
{
    Intern_Entry tmp = *a;
    *a = *b;
    *b = tmp;
}

// e.g: 3 / 4 == 75%, 4 / 5 == 80%, 9 / 10 == 90%
#define LF_NUMERATOR    3
#define LF_DENOMINATOR  4

static Intern_String *
_intern_set(Intern *intern, String text, uint32_t hash)
{
    size_t cap = intern->cap;

    // Adjust by 0.75 load factor but using pure integer math.
    // We do this to ensure there are always empty slots.
    if (intern->count >= (cap * LF_NUMERATOR) / LF_DENOMINATOR) {
        // Always the next power of 2. Unlike dynamic arrays, we always want a
        // new and unique block of memory before we replace the current one.
        size_t new_cap = (cap == 0) ? 1 << 3 : cap << 1;
        _intern_resize(intern, new_cap);
        cap = new_cap;
    }

    // Add 1 for nul terminator.
    Intern_String *value = cast(Intern_String *)mem_rawnew(
        sizeof(*value) + sizeof(value->data[0]) * (text.len + 1),
        alignof(Intern_String), // alignof(*value) is an extension MSVC doesn't have
        intern->allocator
    );

    if (value == NULL)
        return NULL;

    value->len  = text.len;
    value->hash = hash;
    value->data[value->len] = '\0';
    memcpy(value->data, text.data, text.len);

    Intern_Entry  entry   = {.value = value, .probe = 0};
    Intern_Entry *entries = intern->entries;

    // Yes this is basically copy-pasting `_intern_get()`, but we need to add
    // the Robin Hood swapping mechanic.
    for (size_t i = cast(size_t)value->hash % cap; /* empty */; i = (i + 1) % cap) {
        // We can safely write to this entry without overwriting active (live) data?
        if (entries[i].value == NULL) {
            entries[i] = entry;
            ++intern->count;
            return value;
        }

        // Is our current collision "richer" (closer to desired index) than us?
        if (entries[i].probe < entry.probe)
            _intern_swap_entries(&entries[i], &entry);

        _intern_update_max_probe(intern, ++entry.probe);
    }
    __builtin_unreachable();
}

String
intern_get(Intern *intern, String text)
{
    const Intern_String *interned = intern_get_interned(intern, text);
    String key = {interned->data, interned->len};
    return key;
}

const char *
intern_get_cstring(Intern *intern, String text)
{
    // Each interned string is already nul-terminated.
    return intern_get_interned(intern, text)->data;
}

const Intern_String *
intern_get_interned(Intern *intern, String text)
{
    uint32_t      hash  = _intern_hash(text);
    int           probe; // Only needed to avoid NULL checks in `_intern_get()`.
    Intern_Entry *entry = _intern_get(intern->entries, intern->cap, text, hash, &probe);

    // If not yet interned, do so now.
    if (entry == NULL || entry->value == NULL)
        return _intern_set(intern, text, hash);
    else
        return entry->value;
}

#endif // INTERN_IMPLEMENTATION
