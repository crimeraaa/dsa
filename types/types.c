#include "../mem/arena.h"

#include "types.h"
#include "parser.h"

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
    {CType_Kind_Basic,   {{CType_BasicKind_Float,               CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("float complex")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Double,              CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("double complex")}}},
    {CType_Kind_Basic,   {{CType_BasicKind_Long_Double,         CType_BasicFlag_Float | CType_BasicFlag_Complex,    string_literal("long double complex")}}},

    // Misc. Types
    {CType_Kind_Basic,   {{CType_BasicKind_Void,               0,                                                   string_literal("void")}}},
};

Allocator_Error
ctype_table_init(CType_Table *table, Intern *intern, Allocator allocator)
{
    Allocator_Error error;
    CType_Entry    *entries = mem_make(CType_Entry, &error, count_of(ctype_basic_types), allocator);
    if (error)
        return error;

    *table = (CType_Table){
        .allocator = allocator,
        .intern    = intern,
        .entries   = entries,
        .len       = count_of(ctype_basic_types),
        .cap       = count_of(ctype_basic_types),
    };

    // Add all the unqualified basic types
    for (size_t i = 0; i < count_of(ctype_basic_types); ++i) {
        const CType          type = ctype_basic_types[i];
        const Intern_String *name = intern_get_interned(intern, type.basic.name);
        if (name == NULL)
            return Allocator_Error_Out_Of_Memory;

        CType_Info *info = mem_new(CType_Info, &error, allocator);
        if (error)
            return error;

        *info = (CType_Info){
            .name       = name,
            .type       = &ctype_basic_types[i],
            .qualifiers = 0,
            .is_owner   = false,
        };
        entries[i].name = name;
        entries[i].info = info;
    }
    return Allocator_Error_None;
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
        if (info->is_owner) {
            printfln("Freeing '%s'...", info->name->data);
            mem_free(cast(CType *)info->type, allocator);
        }
        mem_free(info, allocator);
    }
    mem_delete(entries, table->cap, allocator);
    table->entries = NULL;
    table->len     = 0;
    table->cap     = 0;
}

static const CType_Info *
_ctype_add(CType_Table *table, CParser *parser, const Intern_String *name)
{
    Allocator allocator = table->allocator;
    size_t    old_cap   = table->cap;
    // Need to resize?
    if (table->len >= old_cap) {
        // NOTE: assume `table->cap` is never 0 by this point!
        size_t          new_cap = old_cap * 2;
        Allocator_Error error;
        CType_Entry    *entries = mem_resize(CType_Entry, &error, table->entries, old_cap, new_cap, allocator);
        if (error)
            return NULL;

        table->entries = entries;
        table->cap     = new_cap;
    }


    Allocator_Error error;
    CType_Info     *info = mem_new(CType_Info, &error, allocator);
    if (error)
        return NULL;

    CType type = parser->data->type;
    if (type.kind == CType_Kind_Basic) {
        *info = (CType_Info){
            .name       = name,
            .type       = &ctype_basic_types[type.basic.kind],
            .qualifiers = parser->data->qualifiers,
            .is_owner   = false,
        };
    } else {
        CType *_type = mem_new(CType, &error, allocator);
        if (error) {
            mem_free(info, allocator);
            return NULL;
        }

        *_type = parser->data->type;
        *info = (CType_Info){
            .name       = name,
            .type       = _type,
            .qualifiers = parser->data->qualifiers,
            .is_owner   = true,
        };
    }

    CType_Entry *entry = &table->entries[table->len++];
    entry->name = name;
    entry->info = info;
    return info;
}

const CType_Info *
ctype_get(CType_Table *table, const char *text, size_t len)
{
    CLexer  lexer  = clexer_make(text, len);
    CParser parser;

    if (!cparser_init(&parser, table, global_temp_allocator))
        return NULL;
    if (!cparser_parse(&parser, &lexer))
        return NULL;

    // WARNING: May not be enough!
    char buf[256];
    String_Builder builder = string_builder_make_fixed(buf, sizeof buf);
    cparser_canonicalize(&parser, &builder);
    const Intern_String *name = intern_get_interned(table->intern, string_to_string(&builder));

    // TODO: Use canonical name as a hash lookup
    CType_Entry *entries = table->entries;
    for (size_t i = 0, len = table->len; i < len; ++i) {
        CType_Entry entry = entries[i];
        if (entry.name == name) {
            return entry.info;
        }
    }
    return _ctype_add(table, &parser, name);
}

void
ctype_table_print(const CType_Table *table)
{
    const CType_Entry *entries = table->entries;
    // TODO: Refactor to be a hashtable
    println("=== TABLE ===");
    for (size_t i = 0, len = table->len; i < len; ++i) {
        const CType_Info *info = entries[i].info;
        const CType      *type = info->type;
        printf("[%zu]: '%s'", i, entries[i].name->data);

        if (type->kind == CType_Kind_Pointer) {
            printfln(" -> '%s'", type->pointer.pointee->name->data);
        } else {
            println("");
        }
    }
    println("=============\n");
}
