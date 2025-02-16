#include "intern.h"

#include <string.h> // memcmp and memcpy, likely highly optimized
#include <stdio.h>  // fprintf

typedef struct {
    size_t   len;
    char     data[];
} Intern_String;

struct Intern_Entry {
    String         key;
    uint32_t       hash;
    int            probe; // Our distance from our ideal position `hash % cap`.
    Intern_String *value;
};

Intern
intern_make(Allocator allocator)
{
    Intern intern;
    intern.allocator = allocator;
    intern.entries   = NULL;
    intern.count     = 0;
    intern.cap       = 0;
    intern.max_probe = 0;
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
_intern_get(Intern_Entry entries[], size_t cap, String string, int *probe)
{
    // Division (and by extension, modulo) by zero is undefined behavior.
    if (cap == 0)
        return NULL;

    uint32_t hash   = fnv_hash(string);
    int      _probe = 0;
    for (size_t i = cast(size_t)hash % cap; /* empty */; ++_probe, i = (i + 1) % cap) {
        // This string isn't interned yet.
        if (entries[i].value == NULL)
            goto set_result;

        // Probably not a match. Keep looking.
        if (entries[i].hash != hash || entries[i].key.len != string.len)
            continue;

        // Confirm if it is indeed a match.
        if (memcmp(string.data, entries[i].key.data, string.len) == 0) set_result: {
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

static void
_intern_resize(Intern *intern, size_t new_cap)
{
    Allocator     allocator   = intern->allocator;
    Intern_Entry *new_entries = mem_make(Intern_Entry, new_cap, allocator);

    // Zero out the new memory so that we can safely read them.
    for (size_t i = 0; i < new_cap; ++i) {
        new_entries[i].key.data = NULL;
        new_entries[i].key.len  = 0;
        new_entries[i].hash     = 0;
        new_entries[i].probe    = 0;
        new_entries[i].value    = NULL;
    }

    // Copy the old non-empty entries.
    size_t        new_count   = 0;
    Intern_Entry *old_entries = intern->entries;
    size_t        old_cap     = intern->cap;

    intern->max_probe = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        // Ignore unset keys; these are yet to be interned.
        if (old_entries[i].value == NULL)
            continue;

        int           probe;
        Intern_Entry *new_entry = _intern_get(new_entries, new_cap, old_entries[i].key, &probe);

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
}

static void
swap(Intern_Entry *a, Intern_Entry *b)
{
    Intern_Entry tmp = *a;
    *a = *b;
    *b = tmp;
}

// e.g: 3 / 4 == 75%, 4 / 5 == 80%, 9 / 10 == 90%
#define LF_NUMERATOR    3
#define LF_DENOMINATOR  4

static Intern_Entry *
_intern_set(Intern *intern, String string)
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
        sizeof(*value) + sizeof(value->data[0]) * (string.len + 1),
        alignof(Intern_String), // alignof(*value) is an extension MSVC doesn't have
        intern->allocator
    );

    value->len = string.len;
    memcpy(value->data, string.data, string.len);
    value->data[string.len] = '\0';

    // VERY important we point to the heap-allocated data, not `string.data`!
    Intern_Entry entry = {
        .key   = {.data = value->data, .len = string.len},
        .hash  = fnv_hash(string),
        .probe = 0,
        .value = value
    };

    Intern_Entry *entries = intern->entries;

    // Yes this is basically copy-pasting `_intern_get()`, but we need to add
    // the Robin Hood swapping mechanic.
    for (size_t i = cast(size_t)entry.hash % cap; /* empty */; i = (i + 1) % cap) {
        // We can safely write to this entry without overwriting active (live) data?
        if (entries[i].value == NULL) {
            entries[i] = entry;
            ++intern->count;
            return &entries[i];
        }

        // Is our current collision "richer" (closer to desired index) than us?
        if (entries[i].probe < entry.probe)
            swap(&entries[i], &entry);

        _intern_update_max_probe(intern, ++entry.probe);
    }
    __builtin_unreachable();
}

String
intern_get(Intern *intern, String string)
{
    int           probe; // Only needed to avoid NULL checks in `_intern_get()`.
    Intern_Entry *entry = _intern_get(intern->entries, intern->cap, string, &probe);
    // If not yet interned, do so now.
    if (entry == NULL || entry->value == NULL)
        entry = _intern_set(intern, string);

    // key already points to the interned value.
    return entry->key;
}

void
intern_print(const Intern *intern, FILE *stream)
{
    const Intern_Entry *entries = intern->entries;
    const size_t        cap     = intern->cap;
    int                *probes  = mem_make(int, cap, intern->allocator);
    int                 totals  = 0;

    memset(probes, 0, sizeof(probes[0]) * cap);

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
            size_t optimal = cast(size_t)entries[i].hash % cap;
            if (optimal != i) {
                fprintf(stream, "; (collision @ %zu, probe: %i)", optimal, entries[i].probe);
                probes[i] = entries[i].probe;
                ++totals;
            }
        }
        fputc('\n', stream);
    }

    fprintf(stream, "\n[COLLISIONS]\n");
    for (size_t i = 0; i < cap; ++i) {
        fprintf(stream, "\t- [%0*zu]: ", count_digits, i);
        for (int j = 0, len = probes[i]; j < len; ++j) {
            fputc('=', stream);
        }
        fputc('\n', stream);
    }

    fprintf(stream,
        "\n[STATISTICS]\n"
        "\t- Count:      %zu\n"
        "\t- Cap:        %zu\n"
        "\t- Unused:     %zu\n"
        "\t- Collisions: %i\n"
        "\t- Max. Probe: %i\n",
        intern->count, cap, cap - intern->count, totals, intern->max_probe);
    mem_delete(probes, cap, intern->allocator);
}
