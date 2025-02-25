#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"
#include "types.h"


#define NEW_SIGNED(type_id)     {.id = (type_id), .modifier = TYPEMOD_SIGNED,   .pointee = NULL,}
#define NEW_UNSIGNED(type_id)   {.id = (type_id), .modifier = TYPEMOD_UNSIGNED, .pointee = NULL,}
#define NEW_TYPE(type_id)       {.id = (type_id), .modifier = TYPEMOD_NONE,     .pointee = NULL,}
#define NEW_COMPLEX(type_id)    {.id = (type_id), .modifier = TYPEMOD_COMPLEX,  .pointee = NULL,}

#define expand   string_expand
#define literal  string_from_literal

static const Type_Info_Table_Entry
TYPES[] = {
    // <type>
    {.name = literal("char"),       .info = NEW_TYPE(TYPEID_CHAR)},
    {.name = literal("short"),      .info = NEW_SIGNED(TYPEID_SHORT)},
    {.name = literal("int"),        .info = NEW_SIGNED(TYPEID_INT)},
    {.name = literal("long"),       .info = NEW_SIGNED(TYPEID_LONG)},
    {.name = literal("long long"),  .info = NEW_SIGNED(TYPEID_LONG_LONG)},
    {.name = literal("float"),      .info = NEW_TYPE(TYPEID_FLOAT)},
    {.name = literal("double"),     .info = NEW_TYPE(TYPEID_DOUBLE)},
    {.name = literal("void"),       .info = NEW_TYPE(TYPEID_VOID)},
    
    // `char` is not guaranteed to be signed or unsigned.
    {.name = literal("signed char"), .info = NEW_SIGNED(TYPEID_CHAR)},
    
    // unsigned types do not alias any other builtin types
    {.name = literal("unsigned char"),      .info = NEW_UNSIGNED(TYPEID_CHAR)},
    {.name = literal("unsigned short"),     .info = NEW_UNSIGNED(TYPEID_SHORT)},
    {.name = literal("unsigned int"),       .info = NEW_UNSIGNED(TYPEID_INT)},
    {.name = literal("unsigned long"),      .info = NEW_UNSIGNED(TYPEID_LONG)},
    {.name = literal("unsigned long long"), .info = NEW_UNSIGNED(TYPEID_LONG_LONG)},
    
    {.name = literal("complex float"),  .info = NEW_COMPLEX(TYPEID_FLOAT)},
    {.name = literal("complex double"), .info = NEW_COMPLEX(TYPEID_DOUBLE)},
};

static void
initialize_types(Type_Info_Table *table)
{
    for (size_t i = 0; i < count_of(TYPES); ++i) {
        String    name = TYPES[i].name;
        Type_Info info = TYPES[i].info;
        type_info_table_add(table, name, info);
    }

    type_info_table_print(table);
}

static bool
set_type_id(Type_Info *info, Type_Id id, String word, String type)
{
    if (string_eq(word, type)) {
        // Was previously set?
        if (info->id != TYPEID_NONE)
            return false;

        info->id = id;
    }
    return true;
}

static bool
set_type_mod(Type_Info *info, Type_Modifier mod, String word, String modstr)
{
    if (string_eq(word, modstr)) {
        // Was previously set?
        if (info->modifier != TYPEMOD_NONE)
            return false;

        info->modifier = mod;
    }
    return true;
}

static void
resolve_type(Type_Info *info)
{
    if (info->id == TYPEID_NONE && info->modifier != TYPEMOD_NONE) {
        switch (info->modifier) {
        // Resolve `signed int` to just `int`.
        case TYPEMOD_SIGNED:
            info->modifier = TYPEMOD_NONE; // fallthrough
        case TYPEMOD_UNSIGNED:
            info->id = TYPEID_INT;
            break;
        case TYPEMOD_COMPLEX:
            info->id = TYPEID_DOUBLE;
            break;
        default:
            break;
        }
    }
    // Resolve `signed short`, `long long signed` to just their types.
    else if (info->id != TYPEID_CHAR && info->modifier == TYPEMOD_SIGNED) {
        info->modifier = TYPEMOD_NONE;
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
    Type_Info info = {.id = TYPEID_NONE, .modifier = TYPEMOD_NONE, .pointee = NULL};
    for (String state = type, word; string_split_any_string_iterator(&state, &word, WHITESPACE);) {
        switch (word.data[0]) {
        case 'c': {
            if (!set_type_id(&info, TYPEID_CHAR, word, literal("char")))
                return NULL;
            if (!set_type_mod(&info, TYPEMOD_COMPLEX, word, literal("complex")))
                return NULL;
            break;
        }
        case 'i':
            if (!set_type_id(&info, TYPEID_INT, word, literal("int"))) {
                // `long int` and `long long int` are valid.
                if (info.id == TYPEID_LONG || info.id == TYPEID_LONG_LONG)
                    continue;
                return NULL;
            }
            break;
        case 'l':
            if (!set_type_id(&info, TYPEID_LONG, word, literal("long"))) {
                // Do not error out on `int long`.
                if (info.id == TYPEID_INT) {
                    info.id = TYPEID_LONG;
                    continue;
                }
                // Do not error out on `long long`.
                else if (info.id == TYPEID_LONG) {
                    info.id = TYPEID_LONG_LONG;
                    continue;
                }
                return NULL;
            }
            break;
        case 's':
            if (!set_type_id(&info, TYPEID_SHORT, word, literal("short")))
                return NULL;
            if (!set_type_mod(&info, TYPEMOD_SIGNED, word, literal("signed")))
                return NULL;
            break;
        case 'u':
            if (!set_type_mod(&info, TYPEMOD_UNSIGNED, word, literal("unsigned")))
                return NULL;
            break;
        case 'v':
            if (!set_type_id(&info, TYPEID_VOID, word, literal("void")))
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
    case TYPEMOD_NONE:
        break;
    case TYPEMOD_SIGNED:
        len += snprintf(start, rem, "signed ");
        break;
    case TYPEMOD_UNSIGNED:
        len += snprintf(start, rem, "unsigned ");
        break;
    case TYPEMOD_COMPLEX:
        len += snprintf(start, rem, "complex ");
        break;
    }
    
    start += len;
    rem   -= cast(size_t)len;

    switch (info.id) {
    case TYPEID_VOID:       len += snprintf(start, rem, "void"); break;
    case TYPEID_CHAR:       len += snprintf(start, rem, "char"); break;
    case TYPEID_SHORT:      len += snprintf(start, rem, "short"); break;
    case TYPEID_INT:        len += snprintf(start, rem, "int"); break;
    case TYPEID_LONG:       len += snprintf(start, rem, "long"); break;
    case TYPEID_LONG_LONG:  len += snprintf(start, rem, "long long"); break;

    case TYPEID_FLOAT:      len += snprintf(start, rem, "float"); break;
    case TYPEID_DOUBLE:     len += snprintf(start, rem, "double"); break;
    case TYPEID_ENUM:       len += snprintf(start, rem, "enum"); break;
    case TYPEID_STRUCT:     len += snprintf(start, rem, "struct"); break;
    case TYPEID_UNION:      len += snprintf(start, rem, "union"); break;
    case TYPEID_POINTER:    len += snprintf(start, rem, "*"); break;
        
    default:
        break;
    }
    
    start += len;
    rem   -= cast(size_t)len;
    
    String query = {buf, cast(size_t)len};
    printfln("Query: " STRING_QFMTSPEC, expand(query));
    return type_info_table_get(table, query);
    // return true;
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
            printfln(STRING_QFMTSPEC ": {pointee = %p, id = %i, modifier = %i}",
                expand(name),
                cast(void *)info->pointee,
                info->id,
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
