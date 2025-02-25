#ifndef TYPES_H
#define TYPES_H

#include "common.h"
#include "allocator.h"
#include "strings.h"
#include "intern.h"

#include <stdint.h>

typedef enum {
    TYPEID_NONE,
    
    // Miscallenous Types: void, <type> *
    // You cannot declare variables of type `void`. Only `void *` is allowed.
    TYPEID_VOID,

    // Fundamental Types: Integers
    TYPEID_CHAR, TYPEID_SHORT, TYPEID_INT, TYPEID_LONG, TYPEID_LONG_LONG,
    
    // Fundamental Types (2): Floating-point
    TYPEID_FLOAT, TYPEID_DOUBLE,
    
    // User-defined Types: structs, enums, unions
    TYPEID_STRUCT, TYPEID_ENUM, TYPEID_UNION,

    // Must query `Type_Info::pointee` for actual type information.
    TYPEID_POINTER,
} Type_Id;

#define TYPEID_COUNT    (TYPEID_POINTER + 1)

extern const String
TYPEID_STRINGS[TYPEID_COUNT];

// Save for `TYPEMOD_NONE`, each of these is mutually exclusive.
typedef enum {
    TYPEMOD_NONE,     // This is the default for most types.
    TYPEMOD_SIGNED,   // Only valid for integer types. Default for `short`, `int`, `long` and `long long`.
    TYPEMOD_UNSIGNED, // Only valid for integer types.
    TYPEMOD_COMPLEX,  // Only valid for floating-point types.
} Type_Modifier;

typedef struct Type_Info Type_Info;
struct Type_Info {
    Type_Id          id;
    Type_Modifier    modifier;
    const Type_Info *pointee; // If non-null, this helps us resolve the pointer type.
};

typedef struct {
    String    name;
    Type_Info info;
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

void
type_info_table_add(Type_Info_Table *table, String name, Type_Info info);

// Return `false` if `name` does not exist to begin with.
bool
type_info_table_alias(Type_Info_Table *table, String name, String alias);

const Type_Info *
type_info_table_get(Type_Info_Table *table, String name);

void
type_info_table_print(const Type_Info_Table *table);

#endif // TYPES_H
