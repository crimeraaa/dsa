#include "types.h"

#include <assert.h>

// NOTE(ORDER): Ensure the order matches `CType_Kind`!
const String
ctype_kind_strings[CType_Kind_Count] = {
    string_literal("CType_Kind_Invalid"),
    string_literal("CType_Kind_Basic"),
    string_literal("CType_Kind_Pointer"),
    string_literal("CType_Kind_Struct"),
    string_literal("CType_Kind_Enum"),
    string_literal("CType_Kind_Union"),
};

// NOTE(ORDER): Ensure the order matches `CType_BasicKind`!
// TODO: Do we care about size?
const CType
ctype_basic_types[CType_BasicKind_Count] = {
    // CType_Kind,       CType_Basic{.kind,                     .flags,                                             .name}
    {CType_Kind_Invalid, {{CType_BasicKind_Invalid,             0,                                                  string_literal("<invalid>")}}},

    // Booleans, Characters
    {CType_Kind_Basic,   {{CType_BasicKind_Bool,                CType_BasicFlag_Bool,                               string_literal("bool")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Char,                CType_BasicFlag_Integer,                            string_literal("char")}}},

    // Signed Integers
    {CType_Kind_Basic,   {{CType_BasicKind_Signed_Char,         CType_BasicFlag_Integer | CType_BasicFlag_Signed,   string_literal("signed char")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Short,               CType_BasicFlag_Integer | CType_BasicFlag_Signed,   string_literal("short")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Int,                 CType_BasicFlag_Integer | CType_BasicFlag_Signed,   string_literal("int")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Long,                CType_BasicFlag_Integer | CType_BasicFlag_Signed,   string_literal("long")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Long_Long,           CType_BasicFlag_Integer | CType_BasicFlag_Signed,   string_literal("long long")}}},

    // Unsigned Integers
    {CType_Kind_Basic,   {{CType_BasicKind_Unsigned_Char,       CType_BasicFlag_Integer | CType_BasicFlag_Unsigned, string_literal("unsigned char")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Unsigned_Short,      CType_BasicFlag_Integer | CType_BasicFlag_Unsigned, string_literal("unsigned short")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Unsigned_Int,        CType_BasicFlag_Integer | CType_BasicFlag_Unsigned, string_literal("unsigned int")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Unsigned_Long,       CType_BasicFlag_Integer | CType_BasicFlag_Unsigned, string_literal("unsigned long")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Unsigned_Long_Long,  CType_BasicFlag_Integer | CType_BasicFlag_Unsigned, string_literal("unsigned long long")}}},

    // Floating-Point Types
    {CType_Kind_Basic,   {{CType_BasicKind_Float,               CType_BasicFlag_Float,                              string_literal("float")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Double,              CType_BasicFlag_Float,                              string_literal("double")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Long_Double,         CType_BasicFlag_Float,                              string_literal("long double")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Float,               CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("complex float")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Double,              CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("complex double")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Long_Double,         CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("complex long double")}}},

    // Misc. Types
    {CType_Kind_Basic,   {{CType_BasicKind_Void,               0,                                                   string_literal("void")}}},
};

Allocator_Error
ctype_table_init(CType_Table *table, Allocator allocator)
{
    Allocator_Error error;
    CType_Entry    *entries = mem_make(CType_Entry, &error, count_of(ctype_basic_types), allocator);
    if (error)
        return error;

    *table = (CType_Table){
        .allocator = allocator,
        .entries   = entries,
        .len       = count_of(ctype_basic_types),
        .cap       = count_of(ctype_basic_types),
    };

    // Add all the unqualified basic types
    for (size_t i = 0; i < count_of(ctype_basic_types); ++i) {
        // CType type = ctype_basic_types[i];
        CType_Info *info = mem_new(CType_Info, &error, allocator);
        if (error)
            return error;

        *info = (CType_Info){
            .type       = &ctype_basic_types[i],
            .qualifiers = 0,
            .is_owner   = false,
        };
        entries[i].info = info;
    }
    return ALLOCATOR_ERROR_NONE;
}

void
ctype_table_destroy(CType_Table *table)
{
    Allocator    allocator = table->allocator;
    CType_Entry *entries   = table->entries;
    for (size_t i = 0, len = table->len; i < len; ++i) {
        CType_Info *info = entries[i].info;
        // the unqualified basic types are allocated in read-only memory, so
        // `info->is_owner` will be false.
        if (info->is_owner)
            mem_free(cast(CType *)info->type, allocator);
        mem_free(info, allocator);
    }
    mem_delete(entries, table->cap, allocator);
    table->entries = NULL;
    table->len     = 0;
    table->cap     = 0;
}

const CType_Info *
ctype_table_get_basic_unqual(CType_Table *table, CType_BasicKind kind)
{
    // `CType_BasicKind_Invalid` and `CType_BasicKind_Count` are not valid.
    assert(CType_BasicKind_Bool <= kind && kind <= CType_BasicKind_Void);
    return table->entries[kind].info;
}

const CType_Info *
ctype_table_get_basic_qual(CType_Table *table, CType_BasicKind kind, CType_QualifierFlag qualifiers)
{
    if (qualifiers == 0)
        return ctype_table_get_basic_unqual(table, kind);

    // Check if this `kind` with these `qualifiers` already exists
    CType_Entry *entries = table->entries;
    for (size_t i = 0, len = table->len; i < len; ++i) {
        const CType_Info *info = entries[i].info;
        // Is this even a basic type with the right qualifiers?
        if (info->type->kind == CType_Kind_Basic && info->qualifiers == qualifiers) {
            // Might be it, but is it the exact one?
            if (info->type->basic.kind == kind)
                return info;
        }
    }
    return NULL;
}

const CType_Info *
ctype_table_add_basic_qual(CType_Table *table, CType_BasicKind kind, CType_QualifierFlag qualifiers)
{
    // Reuse existing entries if at all possible.
    const CType_Info *found = ctype_table_get_basic_qual(table, kind, qualifiers);
    if (found)
        return found;

    size_t old_cap = table->cap;
    // Need to resize?
    if (table->len >= old_cap) {
        // NOTE: assume `table->cap` is never 0 by this point!
        size_t          new_cap = old_cap * 2;
        Allocator_Error error;
        CType_Entry    *entries = mem_resize(CType_Entry, &error, table->entries, old_cap, new_cap, table->allocator);
        if (error)
            return NULL;

        table->entries = entries;
        table->cap     = new_cap;
    }

    Allocator_Error error;
    CType_Info     *info = mem_new(CType_Info, &error, table->allocator);
    if (error)
        return NULL;

    *info = (CType_Info){
        .type       = &ctype_basic_types[kind],
        .qualifiers = qualifiers,
        .is_owner   = false,
    };

    table->entries[table->len++].info = info;
    return info;
}
