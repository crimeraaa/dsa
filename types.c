#include "types.h"

#include <string.h> // memset
#include <stdint.h> // uint32_t

Type_Info_Table
type_info_table_make(Allocator allocator)
{
    Type_Info_Table table = {
        .intern    = intern_make(allocator),
        .entries   = NULL,
        .count     = 0,
        .cap       = 0,
    };
    return table;
}

void
type_info_table_destroy(Type_Info_Table *table)
{
    Intern   *intern    = &table->intern;
    Allocator allocator = intern->allocator;
    
    mem_delete(table->entries, table->cap, allocator);
    intern_destroy(intern);
    table->entries = NULL;
    table->count   = 0;
    table->cap     = 0;
}

#define FNV_OFFSET  2166136261
#define FNV_PRIME   16777619

static uint32_t
fnv_hash(String data)
{
    uint32_t hash = FNV_OFFSET;
    for (size_t i = 0; i < data.len; ++i) {
        // Can't cast the expression to `uint32_t`? Is this not defined behavior?
        hash ^= cast(unsigned char)data.data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

// Finds the first free entry or the entry itself.
// NOTE: Assumes `key` is already interned!
static Type_Info_Table_Entry *
_find_entry(Type_Info_Table_Entry *entries, size_t cap, String key)
{
    // Division (and, by extension, modulo) by 0 is undefined behavior.
    if (cap == 0)
        return NULL;
    
    uint32_t hash = fnv_hash(key);
    
    for (size_t i = cast(size_t)hash % cap; /* empty */; i = (i + 1) % cap) {
        // This entry is free!
        if (entries[i].name.data == NULL)
            return &entries[i];
        
        // We found the entry we were looking for.
        if (string_eq(key, entries[i].name))
            return &entries[i];
    }
    __builtin_unreachable();
}

void
_reserve(Type_Info_Table *table, size_t new_cap)
{
    Intern   *intern    = &table->intern;
    Allocator allocator = intern->allocator;

    Type_Info_Table_Entry *old_entries = table->entries;
    size_t old_cap = table->cap;

    Type_Info_Table_Entry *new_entries = mem_make(Type_Info_Table_Entry, new_cap, allocator);
    
    memset(new_entries, 0, sizeof(new_entries[0]) * new_cap);

    size_t new_count = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        if (old_entries[i].name.len == 0)
            continue;
        
        Type_Info_Table_Entry *entry = _find_entry(new_entries, new_cap, old_entries[i].name);
        *entry = old_entries[i];
        ++new_count;
    }
    
    
    mem_delete(old_entries, old_cap, allocator);
    table->entries = new_entries;
    table->count   = new_count;
    table->cap     = new_cap;

}

void
type_info_table_add(Type_Info_Table *table, String name, Type_Info info)
{
    if (table->count >= (table->cap * 3) / 4) {
        size_t new_cap = (table->cap == 0) ? 8 : table->cap * 2;
        _reserve(table, new_cap);
    }
    
    // Intern now so we can compare pointers directly!
    name = intern_get(&table->intern, name);
    Type_Info_Table_Entry *entry = _find_entry(table->entries, table->cap, name);
    entry->name = name;
    entry->info = info;
    ++table->count;
}

bool
type_info_table_alias(Type_Info_Table *table, String name, String alias)
{
    const Type_Info *info = type_info_table_get(table, name);
    // `name` doesn't exist to begin with!
    if (info == NULL)
        return false;
    
    type_info_table_add(table, alias, *info);
    return true;
}

const Type_Info *
type_info_table_get(Type_Info_Table *table, String name)
{
    // Intern now so we can compare pointers directly!
    name = intern_get(&table->intern, name);
    Type_Info_Table_Entry *entry = _find_entry(table->entries, table->cap, name);
    if (entry == NULL || entry->name.len == 0)
        return NULL;
    
    return &entry->info;
}

void
type_info_table_print(const Type_Info_Table *table)
{
    size_t cap = table->cap;
    println("[TYPE INFO]");
    const Type_Info_Table_Entry *entries = table->entries;
    for (size_t i = 0; i < cap; ++i) {
        printf("\t[%zu]: ", i);
        if (entries[i].name.data == NULL) {
            printf("\n");
            continue;
        }
        printfln(STRING_QFMTSPEC ": {pointee = %p, id = %i, modifier = %i}",
            string_expand(entries[i].name),
            cast(void *)entries[i].info.pointee,
            entries[i].info.id,
            entries[i].info.modifier
        );
    }
}
