#include "types.h"
#include "parser.h"

#define lit string_literal

const String
TYPE_BASE_STRINGS[TYPE_BASE_COUNT] = {
    [TYPE_BASE_NONE]       = {NULL, 0},
    [TYPE_BASE_CHAR]       = lit("char"),
    [TYPE_BASE_SHORT]      = lit("short"),
    [TYPE_BASE_INT]        = lit("int"),
    [TYPE_BASE_LONG]       = lit("long"),
    [TYPE_BASE_LONG_LONG]  = lit("long long"),

    [TYPE_BASE_FLOAT]      = lit("float"),
    [TYPE_BASE_DOUBLE]     = lit("double"),
    [TYPE_BASE_LONG_DOUBLE]= lit("long double"),

    [TYPE_BASE_VOID]       = lit("void"),
    [TYPE_BASE_POINTER]    = {NULL, 0},

    [TYPE_BASE_STRUCT]     = lit("struct"),
    [TYPE_BASE_ENUM]       = lit("enum"),
    [TYPE_BASE_UNION]      = lit("union"),
};

const String
TYPE_MOD_STRINGS[TYPE_MOD_COUNT] = {
    [TYPE_MOD_NONE]     = {NULL, 0},
    [TYPE_MOD_SIGNED]   = lit("signed"),
    [TYPE_MOD_UNSIGNED] = lit("unsigned"),
    [TYPE_MOD_COMPLEX]  = lit("complex"),
};

const String
TYPE_QUAL_STRINGS[TYPE_QUAL_COUNT] = {
    [TYPE_QUAL_CONST]    = lit("const"),
    [TYPE_QUAL_VOLATILE] = lit("volatile"),
    [TYPE_QUAL_RESTRICT] = lit("restrict"),
};

#undef lit

bool
type_parse_string(Type_Table *table, const char *text, size_t len, Allocator allocator)
{
    Error_Handler     handler;
    Allocator_Error   error;
    Type_Parser_Data *data = mem_new(Type_Parser_Data, &error, allocator);
    if (error != ALLOCATOR_ERROR_NONE)
        return false;
    
    data->pointee    = NULL;
    data->base       = TYPE_BASE_NONE;
    data->modifier   = TYPE_MOD_NONE;
    data->qualifiers = 0;

    Type_Parser parser = {
        .allocator  = allocator,
        .lexer      = type_lexer_make(text, len),
        .handler    = &handler,
        .table      = table,
        .data       = data,
    };
    
    handler.error = TYPE_PARSE_NONE;
    if (setjmp(handler.caller) == 0) {
        type_parser_parse(&parser, 1);
        return true;
    } else {
        return false;
    }
}

static const struct {
    Type_Base   base;
    const char *base_name;
} TYPE_INFO_INTEGERS[] = {
    {.base = TYPE_BASE_CHAR,      .base_name = "char"},
    {.base = TYPE_BASE_SHORT,     .base_name = "short"},
    {.base = TYPE_BASE_INT,       .base_name = "int"},
    {.base = TYPE_BASE_LONG,      .base_name = "long"},
    {.base = TYPE_BASE_LONG_LONG, .base_name = "long long"},
};

Type_Table
type_table_make(Allocator allocator)
{
    Type_Table table = {
        .intern  = intern_make(allocator),
        .entries = NULL,
        .len     = 0,
        .cap     = 0,
    };

    for (size_t i = 0; i < count_of(TYPE_INFO_INTEGERS); ++i) {
        Type_Info info = {
            .base    = TYPE_INFO_INTEGERS[i].base,
            .integer = {.modifier = TYPE_MOD_SIGNED, .qualifiers = 0},
        };
        char buf[64];
        String_Builder builder = string_builder_make_fixed(buf, sizeof buf);
        string_append_string(&builder, TYPE_BASE_STRINGS[info.base]);
        if (info.base == TYPE_BASE_CHAR) {
            info.integer.modifier = TYPE_MOD_NONE;
        }
        type_add(&table, string_to_cstring(&builder), info);

        // `signed char` is distinct from `char`.
        string_prepend_char(&builder, ' ');
        string_prepend_string(&builder, TYPE_MOD_STRINGS[info.integer.modifier = TYPE_MOD_SIGNED]);
        if (info.base == TYPE_BASE_CHAR)
            type_add(&table, string_to_cstring(&builder), info);

        // All `unsigned` types are distinct from their base types.
        info.integer.modifier = TYPE_MOD_UNSIGNED;
        string_prepend_cstring(&builder, "un");
        type_add(&table, string_to_cstring(&builder), info);
    }
    type_table_print(&table);
    return table;
}

static void
_print_qualifier(const Type_Info *info, Type_Qualifier qualifier)
{
    if (info->integer.qualifiers & BIT(qualifier)) {
        printf("%s", TYPE_QUAL_STRINGS[qualifier].data);
    }
}

void
type_table_print(const Type_Table *table)
{
    for (size_t i = 0, len = table->len; i < len; ++i) {
        Type_Info *info = table->entries[i].info;
        printf("[%18s] = {modifier = %s, qualifiers = {",
                table->entries[i].base_name->data,
                TYPE_MOD_STRINGS[info->integer.modifier].data);

        _print_qualifier(info, TYPE_QUAL_CONST);
        _print_qualifier(info, TYPE_QUAL_VOLATILE);
        _print_qualifier(info, TYPE_QUAL_RESTRICT);
        println("}}");
    }
}

void
type_table_destroy(Type_Table *table)
{
    Allocator allocator = table->intern.allocator;
    for (size_t i = 0, end = table->len; i < end; ++i) {
        mem_free(table->entries[i].info, allocator);
    }

    mem_delete(table->entries, table->cap, allocator);
    intern_destroy(&table->intern);
    table->entries = NULL;
    table->len     = 0;
    table->cap     = 0;
}

static bool
type_info_eq(const Type_Info *a, const Type_Info *b)
{
    if (a->base != b->base)
        return false;

    switch (a->base) {
    // Integer
    case TYPE_BASE_CHAR:
    case TYPE_BASE_SHORT:
    case TYPE_BASE_INT:
    case TYPE_BASE_LONG:
    case TYPE_BASE_LONG_LONG:
        return a->integer.modifier   == b->integer.modifier
            && a->integer.qualifiers == b->integer.qualifiers;

    // Floating
    case TYPE_BASE_FLOAT:
    case TYPE_BASE_DOUBLE:
    case TYPE_BASE_LONG_DOUBLE:
        return a->floating.modifier   == b->floating.modifier
            && a->floating.qualifiers == b->floating.qualifiers;

    // Misc.
    case TYPE_BASE_VOID:
        break;
    case TYPE_BASE_POINTER:
        return a->pointer.pointee    == b->pointer.pointee
            && a->pointer.qualifiers == b->pointer.qualifiers;

    // User defined
    case TYPE_BASE_STRUCT:
    case TYPE_BASE_ENUM:
    case TYPE_BASE_UNION:
        break;
    default:
        break;
    }
    return false;
}

Allocator_Error
type_add(Type_Table *table, const char *base_name, Type_Info info)
{
    const size_t len = table->len;
    for (size_t i = 0; i < len; ++i) {
        Type_Info *other = table->entries[i].info;
        if (type_info_eq(&info, other))
            return ALLOCATOR_ERROR_NONE;
    }

    Allocator_Error error;
    // Need to grow the dynamic array?
    const size_t cap = table->cap;
    if (len >= cap) {
        size_t            new_cap     = (cap == 0) ? 8 : cap * 2;
        Type_Table_Entry *new_entries = mem_resize(Type_Table_Entry, &error, table->entries, cap, new_cap, table->intern.allocator);
        if (error)
            return error;
        table->entries = new_entries;
        table->cap     = new_cap;
    }

    Type_Info *new_info = mem_new(Type_Info, &error, table->intern.allocator);
    if (error)
        return error;
    *new_info = info;

    Type_Table_Entry *entry = &table->entries[table->len++];
    entry->info      = new_info;
    entry->base_name = intern_get_interned(&table->intern, string_from_cstring(base_name));
    return ALLOCATOR_ERROR_NONE;
}

const Type_Info *
type_get_by_name(Type_Table *table, const char *name)
{
    const Intern_String *query = intern_get_interned(&table->intern, string_from_cstring(name));
    for (size_t i = 0, len = table->len; i < len; ++i) {
        if (query == table->entries[i].base_name) {
            return table->entries[i].info;
        }
    }
    return NULL;
}

const Type_Info *
type_get_by_info(Type_Table *table, Type_Info info)
{
    for (size_t i = 0, len = table->len; i < len; ++i) {
        const Type_Info *other = table->entries[i].info;
        if (type_info_eq(&info, other)) {
            return other;
        }
    }
    return NULL;
}
