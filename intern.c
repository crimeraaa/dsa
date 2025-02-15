#include "intern.h"

#include <string.h> // memcmp and memcpy, likely highly optimized
#include <stdio.h>  // fprintf

Intern
intern_make(Allocator allocator)
{
    Intern intern;
    intern.allocator = allocator;
    intern.entries   = NULL;
    intern.count     = 0;
    intern.cap       = 0;
    return intern;
}

void
intern_destroy(Intern *intern)
{
    Allocator     allocator = intern->allocator;
    Intern_Entry *entries   = intern->entries;
    const size_t  cap       = intern->cap;

    for (size_t i = 0; i < cap; i++) {
        Intern_String *value = entries[i].value;
        if (value == NULL) continue;

        mem_rawfree(value, sizeof(*value) + (value->len + 1), allocator);
    }
    mem_delete(entries, cap, allocator);
    intern->entries = NULL;
    intern->count   = intern->cap = 0;
}

#define FNV_OFFSET  2166136261
#define FNV_PRIME   16777619

static uint32_t
fnv_hash(String data)
{
    uint32_t hash = FNV_OFFSET;
    for (size_t i = 0; i < data.len; i++) {
        // Can't cast the expression to `uint32_t`? Is this not defined behavior?
        hash ^= cast(unsigned char)data.data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

// We pass `entries` directly in the case of `_intern_resize()`.
static Intern_Entry *
_intern_get(Intern_Entry entries[], size_t cap, String string)
{
    // Division and modulo by zero is undefined behavior.
    if (cap == 0) return NULL;

    uint32_t hash = fnv_hash(string);
    for (size_t i = cast(size_t)hash % cap; /* empty */; i = (i + 1) % cap) {
        Intern_Entry  *entry = &entries[i];
        String         key   = entry->key; // Less memory reads this way
        Intern_String *value = entry->value;

        // This string isn't interned yet.
        if (value == NULL) return entry;

        // Probably not a match. Keep looking.
        if (value->hash != hash || key.len != string.len) continue;

        // Confirm if it is indeed a match.
        if (memcmp(string.data, key.data, string.len) == 0) return entry;
    }
    __builtin_unreachable();
}

static void
_intern_resize(Intern *intern, size_t new_cap)
{
    Allocator     allocator   = intern->allocator;
    Intern_Entry *new_entries = mem_make(Intern_Entry, new_cap, allocator);

    // Zero out the new memory so that we can safely read them.
    for (size_t i = 0; i < new_cap; i++) {
        new_entries[i].key.data = NULL;
        new_entries[i].key.len  = 0;
        new_entries[i].value    = NULL;
    }

    // Copy the old entries in the new ones.
    size_t        new_count   = 0;
    Intern_Entry *old_entries = intern->entries;
    size_t        old_cap     = intern->cap;
    for (size_t i = 0; i < old_cap; i++) {
        String         key   = old_entries[i].key;
        Intern_String *value = old_entries[i].value;

        // Ignore unset keys; these are yet to be interned.
        if (value == NULL) continue;

        Intern_Entry *new_entry = _intern_get(new_entries, new_cap, key);
        new_entry->key   = key;
        new_entry->value = value;
        new_count++;
    }

    mem_delete(old_entries, old_cap, allocator);
    intern->entries = new_entries;
    intern->count   = new_count;
    intern->cap     = new_cap;
}

static Intern_Entry *
_intern_set(Intern *intern, String string)
{
    size_t cap = intern->cap;
    // Adjust by 0.75 load factor
    // We do this to ensure there are always empty slots.
    if (intern->count >= (cap * 3) / 4) {
        // We don't need to track the old cap. We do however need the updated
        // value so we can properly do `_intern_get()` later.
        cap = (cap == 0) ? 8 : cap * 2;
        _intern_resize(intern, cap);
    }

    Intern_Entry *entry = _intern_get(intern->entries, cap, string);

    // Add 1 for nul terminator.
    Intern_String *value = cast(Intern_String *)mem_rawnew(
        sizeof(*value) + sizeof(value->data[0]) * (string.len + 1),
        alignof(Intern_String), // alignof(*value) is an extension MSVC doesn't have
        intern->allocator
    );

    value->len  = string.len;
    value->hash = fnv_hash(string);
    memcpy(value->data, string.data, string.len);
    value->data[string.len] = '\0';

    // VERY important we point to the heap-allocated data, not `string.data`!
    String key   = {value->data, string.len};
    entry->key   = key;
    entry->value = value;

    // VERY important we keep track of this.
    intern->count++;
    return entry;
}

String
intern_get(Intern *intern, String string)
{
    Intern_Entry *entry = _intern_get(intern->entries, intern->cap, string);
    // If not yet interned, do so now.
    if (entry == NULL || entry->value == NULL) entry = _intern_set(intern, string);

    // key already points to the interned value.
    return entry->key;
}

void
intern_print(const Intern *intern, FILE *stream)
{
    const Intern_Entry *entries    = intern->entries;
    const size_t        cap        = intern->cap;
    size_t             *collisions = mem_make(size_t, cap, intern->allocator);
    size_t              totals     = 0;

    memset(collisions, 0, sizeof(collisions[0]) * cap);

    int count_digits = (cap == 0) ? 1 : 0;
    for (size_t tmp = cap; tmp != 0; tmp /= 10) {
        ++count_digits;
    }

    fprintf(stream, "[INTERNED]\n");
    for (size_t i = 0; i < cap; ++i) {
        const Intern_String *value = entries[i].value;

        fprintf(stream, "\t- [%0*zu]:", count_digits, i);
        if (value != NULL) {
            fprintf(stream, " \"%s\"", value->data);
            size_t optimal = cast(size_t)value->hash % cap;
            if (optimal != i) {
                fprintf(stream, "; (collision @ %zu)", optimal);
                ++collisions[optimal];
                ++totals;
            }
        }
        fputc('\n', stream);
    }

    fprintf(stream, "\n[COLLISIONS]\n");
    for (size_t i = 0; i < cap; ++i) {
        fprintf(stream, "\t- [%0*zu]: ", count_digits, i);
        for (size_t j = 0, len = collisions[i]; j < len; ++j) {
            fputc('=', stream);
        }
        fputc('\n', stream);
    }

    fprintf(stream,
        "\n[STATISTICS]\n"
        "\t- Count:      %zu\n"
        "\t- Cap:        %zu\n"
        "\t- Unused:     %zu\n"
        "\t- Collisions: %zu\n",
        intern->count, cap, cap - intern->count, totals);
    mem_delete(collisions, cap, intern->allocator);
}
