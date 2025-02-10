#include "intern.h"

#include <string.h> // memcmp and memcpy, likely highly optimized

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
        Intern_Entry *entry = &entries[i];
        Intern_String value = entry->value;
        if (value == NULL) continue;
        
        mem_rawfree(value, sizeof(*value) + (value->len + 1), allocator);
    }
    mem_delete(entries, cap, allocator);
    intern->entries = NULL;
    intern->count   = intern->cap = 0;
}

#define FNV1A_OFFSET_32 2166136261
#define FNV1A_PRIME_32  16777619

static uint32_t
fnv1a_hash(String data)
{
    uint32_t hash = FNV1A_OFFSET_32;
    for (int i = 0; i < data.len; i++) {
        // Can't cast the expression to `uint32_t`? Is this not defined behavior?
        hash ^= cast(unsigned char)data.data[i];
        hash *= FNV1A_PRIME_32;
    }
    return hash;
}

// We pass `entries` directly in the case of `_intern_adjust_capacity()`.
static Intern_Entry *
_intern_get(Intern_Entry entries[], size_t cap, String string)
{
    uint32_t hash = fnv1a_hash(string);
    
    // Division and modulo by zero is undefined behavior.
    if (cap == 0) return NULL;

    for (size_t i = cast(size_t)hash % cap; /* empty */; i = (i + 1) % cap) {
        Intern_Entry *entry = &entries[i];
        Intern_String value = entry->value;
        
        // This string isn't interned yet.
        if (value == NULL) return entry;
        
        // Probably not a match. Keep looking.
        if (value->hash != hash || value->len != cast(size_t)string.len) continue;

        // Confirm if it is indeed a match.
        if (memcmp(string.data, value->data, value->len) == 0) {
            return entry;
        }
    }
    __builtin_unreachable();
}

static void
_intern_adjust_capacity(Intern *intern, size_t new_cap)
{
    Allocator     allocator   = intern->allocator;
    Intern_Entry *new_entries = mem_make(Intern_Entry, new_cap, allocator);
    
    // Zero out the new memory.
    for (size_t i = 0; i < new_cap; i++) {
        new_entries[i].key.data = NULL;
        new_entries[i].key.len  = 0;
        new_entries[i].value    = NULL;
    }
    
    // Copy the old entries in the new ones.
    size_t new_count = 0;
    for (size_t i = 0; i < intern->cap; i++) {
        Intern_Entry  old_entry = intern->entries[i];
        String        key       = old_entry.key;
        Intern_String value     = old_entry.value;

        // Ignore unset keys; these are yet to be interned.
        if (value == NULL) continue;
        
        Intern_Entry *new_entry = _intern_get(new_entries, new_cap, key);
        new_entry->key   = key;
        new_entry->value = value;
        new_count++;
    }
    
    mem_delete(intern->entries, intern->cap, allocator);
    intern->entries = new_entries;
    intern->count   = new_count;
    intern->cap     = new_cap;
}

// Sets `out_entry` to point to the interned version of `string`.
static Intern_Entry *
_intern_set(Intern *intern, String string)
{
    size_t cap = intern->cap;
    // Adjust by 0.75 load factor
    // We do this to ensure there are always empty slots.
    if (intern->count >= (cap * 3) / 4) {
        size_t new_cap = (cap == 0) ? 8 : cap * 2;
        _intern_adjust_capacity(intern, new_cap);
        // Needed so we can properly do `_intern_get()` later.
        cap = new_cap;
    }
    
    Intern_Entry *entry = _intern_get(intern->entries, cap, string);
    const size_t  len   = cast(size_t)string.len;
    
    // Add 1 for nul terminator.
    Intern_String value = cast(Intern_String)mem_rawnew(
        sizeof(*value) + (len + 1),
        alignof(Intern_String), // alignof(*value) is an extension
        intern->allocator);
    value->len  = len;
    value->hash = fnv1a_hash(string);
    memcpy(value->data, string.data, len);
    value->data[len] = '\0';
    
    String key   = {value->data, cast(int)len};
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
    if (entry == NULL || entry->value == NULL) {
        entry = _intern_set(intern, string);
    }
    // key already points to the interned value.
    return entry->key;
}
