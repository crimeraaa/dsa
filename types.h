#ifndef TYPES_H
#define TYPES_H

#include "common.h"
#include "allocator.h"
#include "strings.h"
#include "intern.h"

#include <stdint.h>

typedef enum {
    // Mainly used for `signed`, `unsigned` and `complex` when used as-is.
    // Notably, this is distinct from `void`.
    TYPE_BASE_NONE,

    // Miscallenous Types: void
    // You cannot declare variables of type `void`. Only `void *` is allowed.
    TYPE_BASE_VOID,

    // Fundamental Types: Integers
    TYPE_BASE_CHAR, TYPE_BASE_SHORT, TYPE_BASE_INT, TYPE_BASE_LONG, TYPE_BASE_LONG_LONG,

    // Fundamental Types (2): Floating-point
    TYPE_BASE_FLOAT, TYPE_BASE_DOUBLE,

    // User-defined Types: structs, enums, unions
    TYPE_BASE_STRUCT, TYPE_BASE_ENUM, TYPE_BASE_UNION,

    // Must query `Type_Info::pointee` for actual type information.
    TYPE_BASE_POINTER,
} Type_Base;

// Save for `TYPE_MOD_NONE`, each of these is mutually exclusive.
typedef enum {
    TYPE_MOD_NONE,     // This is the default for most types.
    TYPE_MOD_SIGNED,   // Only valid for integer types. Default for `short`, `int`, `long` and `long long`.
    TYPE_MOD_UNSIGNED, // Only valid for integer types.
    TYPE_MOD_COMPLEX,  // Only valid for floating-point types.
} Type_Modifier;

#define TYPE_BASE_COUNT (TYPE_BASE_POINTER + 1)
#define TYPE_MOD_COUNT  (TYPE_MOD_COMPLEX + 1)

extern const String
TYPE_BASE_STRINGS[TYPE_BASE_COUNT],
TYPE_MOD_STRINGS[TYPE_MOD_COUNT];

typedef struct Type_Info Type_Info;
struct Type_Info {
    Type_Base     base;
    Type_Modifier modifier;
};

typedef struct {
    const Intern_String *name;
    Type_Info            info;
} Type_Info_Table_Entry;

typedef struct {
    Intern                 intern; // Also contains the allocator.
    Type_Info_Table_Entry *entries;
    size_t                 count;
    size_t                 cap;
} Type_Info_Table;

Type_Info_Table
type_info_table_make(Allocator allocator);

void
type_info_table_destroy(Type_Info_Table *table);

bool
type_info_table_add(Type_Info_Table *table, String name, Type_Info info);

bool
type_info_table_new_alias(Type_Info_Table *table, String name, String alias);

bool
type_info_table_get(Type_Info_Table *table, String name, Type_Info *out_info);

void
type_info_table_print(const Type_Info_Table *table, FILE *stream);

#endif // TYPES_H
