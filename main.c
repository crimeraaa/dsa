#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"
#include "types.h"


#define NEW_SIGNED(type_base)   {.base = (type_base), .modifier = TYPE_MOD_SIGNED,   .pointee = NULL,}
#define NEW_UNSIGNED(type_base) {.base = (type_base), .modifier = TYPE_MOD_UNSIGNED, .pointee = NULL,}
#define NEW_TYPE(type_base)     {.base = (type_base), .modifier = TYPE_MOD_NONE,     .pointee = NULL,}
#define NEW_COMPLEX(type_base)  {.base = (type_base), .modifier = TYPE_MOD_COMPLEX,  .pointee = NULL,}

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
set_type_id(Type_Info *info, Type_Base base, String word, String type)
{
    if (string_eq(word, type)) {
        // Was previously set?
        if (info->base != TYPE_BASE_NONE)
            return false;

        info->base = base;
    }
    return true;
}

static bool
set_type_mod(Type_Info *info, Type_Modifier mod, String word, String modstr)
{
    if (string_eq(word, modstr)) {
        // Was previously set?
        if (info->modifier != TYPE_MOD_NONE)
            return false;

        info->modifier = mod;
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

static const Type_Info *
parse_type(Type_Info_Table *table, String type)
{
    // This type already exists and is valid?
    const Type_Info *_info = type_info_table_get(table, type);
    if (_info != NULL)
        return _info;

    // Parse the string `type` and generate a query from it.
    Type_Info info = {.base = TYPE_BASE_NONE, .modifier = TYPE_MOD_NONE, .pointee = NULL};
    for (String state = type, word; string_split_any_string_iterator(&state, &word, WHITESPACE);) {
        switch (word.data[0]) {
        case 'c': {
            if (!set_type_id(&info, TYPE_BASE_CHAR, word, literal("char")))
                return NULL;
            if (!set_type_mod(&info, TYPE_MOD_COMPLEX, word, literal("complex")))
                return NULL;
            break;
        }
        case 'i':
            if (!set_type_id(&info, TYPE_BASE_INT, word, literal("int"))) {
                // `long int` and `long long int` are valid.
                if (info.base == TYPE_BASE_LONG || info.base == TYPE_BASE_LONG_LONG)
                    continue;
                return NULL;
            }
            break;
        case 'l':
            if (!set_type_id(&info, TYPE_BASE_LONG, word, literal("long"))) {
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
            if (!set_type_id(&info, TYPE_BASE_SHORT, word, literal("short")))
                return NULL;
            if (!set_type_mod(&info, TYPE_MOD_SIGNED, word, literal("signed")))
                return NULL;
            break;
        case 'u':
            if (!set_type_mod(&info, TYPE_MOD_UNSIGNED, word, literal("unsigned")))
                return NULL;
            break;
        case 'v':
            if (!set_type_id(&info, TYPE_BASE_VOID, word, literal("void")))
                return NULL;
            break;
        }
    }
    
    
    char   buf[512];
    char  *start = buf;
    size_t rem   = sizeof(buf);
    int    len   = 0;
    
    resolve_type(&info);
    switch (info.modifier) {
    case TYPE_MOD_NONE:
        break;
    case TYPE_MOD_SIGNED:
        len += snprintf(start, rem, "signed ");
        break;
    case TYPE_MOD_UNSIGNED:
        len += snprintf(start, rem, "unsigned ");
        break;
    case TYPE_MOD_COMPLEX:
        len += snprintf(start, rem, "complex ");
        break;
    }
    
    start += len;
    rem   -= cast(size_t)len;

    switch (info.base) {
    case TYPE_BASE_VOID:       len += snprintf(start, rem, "void"); break;
    case TYPE_BASE_CHAR:       len += snprintf(start, rem, "char"); break;
    case TYPE_BASE_SHORT:      len += snprintf(start, rem, "short"); break;
    case TYPE_BASE_INT:        len += snprintf(start, rem, "int"); break;
    case TYPE_BASE_LONG:       len += snprintf(start, rem, "long"); break;
    case TYPE_BASE_LONG_LONG:  len += snprintf(start, rem, "long long"); break;

    case TYPE_BASE_FLOAT:      len += snprintf(start, rem, "float"); break;
    case TYPE_BASE_DOUBLE:     len += snprintf(start, rem, "double"); break;
    case TYPE_BASE_ENUM:       len += snprintf(start, rem, "enum"); break;
    case TYPE_BASE_STRUCT:     len += snprintf(start, rem, "struct"); break;
    case TYPE_BASE_UNION:      len += snprintf(start, rem, "union"); break;
    case TYPE_BASE_POINTER:    len += snprintf(start, rem, "*"); break;
        
    default:
        break;
    }
    
    String query = {buf, cast(size_t)len};
    printfln("Query: " STRING_QFMTSPEC, expand(query));
    return type_info_table_get(table, query);
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
        
        String tmp  = {.data = buf, .len = strcspn(buf, "\r\n")};
        String name = string_trim_space(tmp);
        const Type_Info *info = parse_type(table, name);
        if (info != NULL) {
            printfln(STRING_QFMTSPEC ": {pointee = %p, base = %i, modifier = %i}",
                expand(name),
                cast(void *)info->pointee,
                info->base,
                info->modifier
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
    
    Type_Info_Table table = type_info_table_make(HEAP_ALLOCATOR);
    initialize_types(&table);
    run_interactive(&table);
    type_info_table_destroy(&table);
    
    return 0;
}
