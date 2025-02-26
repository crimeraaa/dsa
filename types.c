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

// Finds the first free entry or the entry itself.
static Type_Info_Table_Entry *
_find_entry(Type_Info_Table_Entry *entries, size_t cap, const Intern_String *key)
{
    // Division (and, by extension, modulo) by 0 is undefined behavior.
    if (cap == 0)
        return NULL;
    
    for (size_t i = cast(size_t)key->hash % cap; /* empty */; i = (i + 1) % cap) {
        // This entry is free!
        if (entries[i].name == NULL)
            return &entries[i];
        
        // We found the entry we were looking for.
        if (key == entries[i].name)
            return &entries[i];
    }
    __builtin_unreachable();
}

void
_reserve(Type_Info_Table *table, size_t new_cap)
{
    Allocator allocator = table->intern.allocator;

    Type_Info_Table_Entry *old_entries = table->entries;
    size_t old_cap = table->cap;

    Type_Info_Table_Entry *new_entries = mem_make(Type_Info_Table_Entry, new_cap, allocator);
    
    memset(new_entries, 0, sizeof(new_entries[0]) * new_cap);

    size_t new_count = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        if (old_entries[i].name == NULL)
            continue;
        
        *_find_entry(new_entries, new_cap, old_entries[i].name) = old_entries[i];
        ++new_count;
    }
    
    
    mem_delete(old_entries, old_cap, allocator);
    table->entries = new_entries;
    table->count   = new_count;
    table->cap     = new_cap;

}

const Type_Info *
type_info_table_add(Type_Info_Table *table, String name, Type_Info info)
{
    if (table->count >= (table->cap * 3) / 4) {
        size_t new_cap = (table->cap == 0) ? 8 : table->cap * 2;
        _reserve(table, new_cap);
    }
    
    const Intern_String   *key   = intern_get_interned(&table->intern, name);
    Type_Info_Table_Entry *entry = _find_entry(table->entries, table->cap, key);
    ++table->count;
    entry->name = key;
    entry->info = info;
    return &entry->info;
}

const Type_Info *
type_info_table_new_alias(Type_Info_Table *table, String name, String alias)
{
    const Type_Info *info = type_info_table_get(table, name);
    // `name` doesn't exist to begin with?
    if (info == NULL)
        return NULL;
    else
        return type_info_table_add(table, alias, *info);
}

const Type_Info *
type_info_table_get(Type_Info_Table *table, String name)
{
    const Intern_String   *key   = intern_get_interned(&table->intern, name);
    Type_Info_Table_Entry *entry = _find_entry(table->entries, table->cap, key);
    if (entry == NULL || entry->name == NULL)
        return NULL;
    else
        return &entry->info;
}

void
type_info_table_print(const Type_Info_Table *table, FILE *stream)
{
    size_t cap = table->cap;
    fprintln(stream, "[TYPE INFO]");
    const Type_Info_Table_Entry *entries = table->entries;
    for (size_t i = 0; i < cap; ++i) {
        fprintf(stream, "\t[%zu]: ", i);
        if (entries[i].name == NULL) {
            fputc('\n', stream);
            continue;
        }
        fprintfln(stream, "\"%s\": {pointee = %p, base = %i, modifier = %i}",
            entries[i].name->data,
            cast(void *)entries[i].info.pointee,
            entries[i].info.base,
            entries[i].info.modifier
        );
    }
}
