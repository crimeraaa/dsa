#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"
#include "types.h"


#define NEW_SIGNED(type_base)   {.base = (type_base), .modifier = TYPE_MOD_SIGNED}
#define NEW_UNSIGNED(type_base) {.base = (type_base), .modifier = TYPE_MOD_UNSIGNED}
#define NEW_TYPE(type_base)     {.base = (type_base), .modifier = TYPE_MOD_NONE}
#define NEW_COMPLEX(type_base)  {.base = (type_base), .modifier = TYPE_MOD_COMPLEX}

#define expand   string_expand
#define literal  string_literal

static const struct {
    const char *name;
    Type_Info   info;
} TYPES[] = {
    // <type>
    {.name = "char",                .info = NEW_TYPE(TYPE_BASE_CHAR)},
    {.name = "short",               .info = NEW_SIGNED(TYPE_BASE_SHORT)},
    {.name = "int",                 .info = NEW_SIGNED(TYPE_BASE_INT)},
    {.name = "long",                .info = NEW_SIGNED(TYPE_BASE_LONG)},
    {.name = "long long",           .info = NEW_SIGNED(TYPE_BASE_LONG_LONG)},
    {.name = "float",               .info = NEW_TYPE(TYPE_BASE_FLOAT)},
    {.name = "double",              .info = NEW_TYPE(TYPE_BASE_DOUBLE)},
    {.name = "void",                .info = NEW_TYPE(TYPE_BASE_VOID)},
    
    // `char` is not guaranteed to be signed or unsigned.
    {.name = "signed char",         .info = NEW_SIGNED(TYPE_BASE_CHAR)},
    
    // unsigned types do not alias any other builtin types
    {.name = "unsigned char",       .info = NEW_UNSIGNED(TYPE_BASE_CHAR)},
    {.name = "unsigned short",      .info = NEW_UNSIGNED(TYPE_BASE_SHORT)},
    {.name = "unsigned int",        .info = NEW_UNSIGNED(TYPE_BASE_INT)},
    {.name = "unsigned long",       .info = NEW_UNSIGNED(TYPE_BASE_LONG)},
    {.name = "unsigned long long",  .info = NEW_UNSIGNED(TYPE_BASE_LONG_LONG)},
    
    {.name = "complex float",       .info = NEW_COMPLEX(TYPE_BASE_FLOAT)},
    {.name = "complex double",      .info = NEW_COMPLEX(TYPE_BASE_DOUBLE)},
};

static void
initialize_types(Type_Info_Table *table)
{
    for (size_t i = 0; i < count_of(TYPES); ++i) {
        String    name = {TYPES[i].name, strlen(TYPES[i].name)};
        Type_Info info = TYPES[i].info;
        type_info_table_add(table, name, info);
    }

    type_info_table_print(table, stdout);
}

static bool
set_type_id(Type_Info *info, Type_Base base, String word)
{
    if (string_eq(word, TYPE_BASE_STRINGS[base])) {
        // Was previously set?
        if (info->base != TYPE_BASE_NONE)
            return false;

        info->base = base;
    }
    return true;
}

static bool
set_type_mod(Type_Info *info, Type_Modifier modifier, String word)
{
    if (string_eq(word, TYPE_MOD_STRINGS[modifier])) {
        // Was previously set?
        if (info->modifier != TYPE_MOD_NONE)
            return false;

        info->modifier = modifier;
    }
    return true;
}

static void
resolve_type(Type_Info *info)
{
    if (info->base == TYPE_BASE_NONE && info->modifier != TYPE_MOD_NONE) {
        switch (info->modifier) {
        // Resolve `signed` to `int` and `unsigned` to `unsigned int`.
        case TYPE_MOD_SIGNED:
            info->modifier = TYPE_MOD_NONE; // fallthrough
        case TYPE_MOD_UNSIGNED:
            info->base = TYPE_BASE_INT;
            break;
        case TYPE_MOD_COMPLEX:
            info->base = TYPE_BASE_DOUBLE;
            break;
        default:
            break;
        }
    }
    // Resolve `signed short`, `long long signed` to just their types.
    else if (info->base != TYPE_BASE_CHAR && info->modifier == TYPE_MOD_SIGNED) {
        info->modifier = TYPE_MOD_NONE;
    }
}

#define WHITESPACE  literal(" \r\n\t\v\f")

static bool
parse_type(Type_Info_Table *table, String type, Type_Info *out_info)
{
    // This type already exists and is valid?
    if (type_info_table_get(table, type, out_info))
        return true;

    // Parse the string `type` and generate a query from it.
    Type_Info info = NEW_TYPE(TYPE_BASE_NONE);
    for (String state = type, word; string_split_any_string_iterator(&state, &word, WHITESPACE);) {
        switch (word.data[0]) {
        case 'c': {
            if (!set_type_id(&info, TYPE_BASE_CHAR, word))
                return NULL;
            if (!set_type_mod(&info, TYPE_MOD_COMPLEX, word))
                return NULL;
            break;
        }
        case 'i':
            if (!set_type_id(&info, TYPE_BASE_INT, word)) {
                // `long int` and `long long int` are valid.
                if (info.base == TYPE_BASE_LONG || info.base == TYPE_BASE_LONG_LONG)
                    continue;
                return NULL;
            }
            break;
        case 'l':
            if (!set_type_id(&info, TYPE_BASE_LONG, word)) {
                // Do not error out on `int long`.
                if (info.base == TYPE_BASE_INT) {
                    info.base = TYPE_BASE_LONG;
                    continue;
                }
                // Do not error out on `long long`.
                else if (info.base == TYPE_BASE_LONG) {
                    info.base = TYPE_BASE_LONG_LONG;
                    continue;
                }
                return NULL;
            }
            break;
        case 's':
            if (!set_type_id(&info, TYPE_BASE_SHORT, word))
                return NULL;
            if (!set_type_mod(&info, TYPE_MOD_SIGNED, word))
                return NULL;
            break;
        case 'u':
            if (!set_type_mod(&info, TYPE_MOD_UNSIGNED, word))
                return NULL;
            break;
        case 'v':
            if (!set_type_id(&info, TYPE_BASE_VOID, word))
                return NULL;
            break;
        }
    }
    
    
    char buf[512];
    String_Builder builder = string_builder_make_fixed(buf, sizeof buf);
    resolve_type(&info);
    string_builder_append_string(&builder, TYPE_MOD_STRINGS[info.modifier]);
    
    // If we had `TYPE_BASE_NONE` with `TYPE_MOD_SIGNED` (e.g. "signed"), we
    // should have resolved to `TYPE_BASE_INT` with `TYPE_MOD_NONE`.
    if (info.modifier != TYPE_MOD_NONE)
        string_builder_append_char(&builder, ' ');
    string_builder_append_string(&builder, TYPE_BASE_STRINGS[info.base]);

    // Testing if we already nul-terminated it
    const char *query = string_builder_to_cstring(&builder);
    printfln("Query: \'%s\'", query);
    return type_info_table_get(table, string_from_cstring(query), out_info);
}

static void
run_interactive(Type_Info_Table *table)
{
    char buf[512];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }
        
        String    tmp  = {.data = buf, .len = strcspn(buf, "\r\n")};
        String    name = string_trim_space(tmp);
        Type_Info info;
        if (parse_type(table, name, &info)) {
            printfln(STRING_QFMTSPEC ": {base = %i, modifier = %i}",
                expand(name),
                info.base,
                info.modifier
            );
        } else {
            printfln("Invalid type " STRING_QFMTSPEC ".", expand(name));
        }
    }
}


int
main(int argc, char *argv[])
{
    unused(argc);
    unused(argv);
    
    Type_Info_Table table = type_info_table_make(PANIC_ALLOCATOR);
    initialize_types(&table);
    run_interactive(&table);
    type_info_table_destroy(&table);
    
    return 0;
}
