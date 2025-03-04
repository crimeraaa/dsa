#pragma once

#include "common.h"
#include "strings.h"
#include "intern.h"

/**
 * @brief
 *      'basic' types are the first step in determining what a type even *is*.
 *
 * @note
 *      We do not consider modifiers such as `signed`, `unsigned`, `complex`,
 *      `const`, etc. We only want to check how we should begin interpreting this
 *      type.
 */
enum Type_Base {
    TYPE_BASE_NONE,        // The default. May also signal invalid types.
    TYPE_BASE_CHAR,        // @note Not guaranteed to be `signed` nor `unsigned`.
    TYPE_BASE_SHORT,       // aliases: `short int` and `int short` and `signed` variationos thereof.
    TYPE_BASE_INT,         // aliases: `signed int` and `int signed`.
    TYPE_BASE_LONG,        // aliases: `long int`, `int long` and `signed` variations thereof.
    TYPE_BASE_LONG_LONG,   // aliases: `long long int`, `int long long`, `long int long` and `signed` variations thereof.

    TYPE_BASE_FLOAT,
    TYPE_BASE_DOUBLE,
    TYPE_BASE_LONG_DOUBLE, // ugh

    TYPE_BASE_VOID,
    TYPE_BASE_POINTER,

    TYPE_BASE_STRUCT,
    TYPE_BASE_ENUM,
    TYPE_BASE_UNION,

    TYPE_BASE_COUNT,
};
typedef enum Type_Base Type_Base;

enum Type_Modifier {
    TYPE_MOD_NONE,
    TYPE_MOD_SIGNED,
    TYPE_MOD_UNSIGNED,
    TYPE_MOD_COMPLEX,
    TYPE_MOD_COUNT,
};
typedef enum Type_Modifier Type_Modifier;

#define BIT(N)  (1 << (N))

/**
 * @note
 *      Qualifiers can be nested, e.g. `const volatile int`.
 *      So each enumeration member (sans `TYPE_QUAL_COUNT`) actually represents
 *      a bit flag. Use the `BIT` macro to get them.
 */
typedef uint8_t             Type_Qualifier_Set;

enum Type_Qualifier {
    TYPE_QUAL_CONST,
    TYPE_QUAL_VOLATILE,
    TYPE_QUAL_RESTRICT,
    TYPE_QUAL_COUNT,
};
typedef enum Type_Qualifier Type_Qualifier;

typedef struct Type_Table           Type_Table;
typedef struct Type_Table_Entry     Type_Table_Entry;
typedef struct Type_Info            Type_Info;
typedef struct Type_Info_Integer    Type_Info_Integer;
typedef struct Type_Info_Floating   Type_Info_Floating;
typedef struct Type_Info_Pointer    Type_Info_Pointer;

struct Type_Info_Pointer {
    Type_Info         *pointee;
    Type_Qualifier_Set qualifiers;
};

struct Type_Info_Integer {
    Type_Modifier      modifier;    // none, signed or unsigned.
    Type_Qualifier_Set qualifiers;  // const or volatile.
};

struct Type_Info_Floating {
    Type_Modifier      modifier;    // none or complex.
    Type_Qualifier_Set qualifiers;  // const or volatile.
};

struct Type_Info {
    Type_Base basic;
    union {
        Type_Info_Integer   integer;
        Type_Info_Floating  floating;
        Type_Info_Pointer   pointer;
    };
};

struct Type_Table_Entry {
    const Intern_String *base_name;
    Type_Info           *info;
};

struct Type_Table {
    Intern            intern;   // Also contains the allocator.
    Type_Table_Entry *entries;
    size_t            len;
    size_t            cap;
};

Type_Table
type_table_make(Allocator allocator);

void
type_table_destroy(Type_Table *table);

void
type_table_print(const Type_Table *table);

Allocator_Error
type_add(Type_Table *table, const char *base_name, Type_Info info);

const Type_Info *
type_get_by_name(Type_Table *table, const char *name);

const Type_Info *
type_get_by_info(Type_Table *table, Type_Info info);

bool
type_parse_string(Type_Table *table, const char *text, size_t len);
