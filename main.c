#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"
#include "types.h"


#define NEW_SIGNED(type_id)     {.id = (type_id), .modifier = TYPEMOD_SIGNED,   .pointee = NULL,}
#define NEW_UNSIGNED(type_id)   {.id = (type_id), .modifier = TYPEMOD_UNSIGNED, .pointee = NULL,}
#define NEW_TYPE(type_id)       {.id = (type_id), .modifier = TYPEMOD_NONE,     .pointee = NULL,}

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
};

static const struct {
    String name, alias;
} ALIASES[] = {
    // https://stackoverflow.com/questions/44624857/is-int-always-signed
    {.name = literal("int"),       .alias = literal("signed")},
    {.name = literal("int"),       .alias = literal("signed int")},
    {.name = literal("int"),       .alias = literal("int signed")},

    // <size> int
    {.name = literal("short"),     .alias = literal("short int")},
    {.name = literal("long"),      .alias = literal("long int")},
    {.name = literal("long long"), .alias = literal("long long int")},
    
    // int <size>
    {.name = literal("short"),      .alias = literal("int short")},
    {.name = literal("long"),       .alias = literal("int long")},
    {.name = literal("long long"),  .alias = literal("int long long")},
};

static void
initialize_types(Type_Info_Table *table)
{
    for (size_t i = 0; i < count_of(TYPES); ++i) {
        String    name = TYPES[i].name;
        Type_Info info = TYPES[i].info;
        type_info_table_add(table, name, info);
    }

    for (size_t i = 0; i < count_of(ALIASES); ++i) {
        type_info_table_alias(table, ALIASES[i].name, ALIASES[i].alias);
    }
    type_info_table_print(table);
}

// Type_Info
// parse_type(Type_Info_Table *table, String type)
// {
//     Type_Info info = {.pointee = NULL, .id = TYPEID_VOID, .modifier = TYPEMOD_NONE};
//     size_t ptr_index = string_index_char(type, '*');
//     if (ptr_index != STRING_NOT_FOUND) {
//         // You can't possibly declare something like `* int x;`.
//         if (ptr_index == 0) {
//             return info;
//         }
//         // e.g. `"const char"` from `"const char *"`.
//         String pointee_name = string_trim_space(string_slice(type, 0, ptr_index - 1));
//     }
//     // "signed" or "unsigned"?
//     size_t mod_index = string_index_subcstring(type, "signed");
//     return info;
// }

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
        
        String tmp = {.data = buf, .len = strcspn(buf, "\r\n")};
        String tmp2 = string_trim_space(tmp);
        printfln(STRING_QFMTSPEC " => " STRING_QFMTSPEC, expand(tmp), expand(tmp2));
        String name = intern_get(&table->intern, tmp);
        const Type_Info *info = type_info_table_get(table, name);
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
