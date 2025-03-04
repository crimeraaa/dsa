#pragma once

#include "common.h"
#include "strings.h"

/**
 * @brief
 *      'basic' types are the first step in determining what a type even *is*.
 *
 * @note
 *      We do not consider modifiers such as `signed`, `unsigned`, `complex`,
 *      `const`, etc. We only want to check how we should begin interpreting this
 *      type.
 */
typedef enum Type_Basic Type_Basic;
enum Type_Basic {
    TYPE_BASIC_NONE,        // The default. May also signal invalid types.
    TYPE_BASIC_CHAR,        // @note Not guaranteed to be `signed` nor `unsigned`.
    TYPE_BASIC_SHORT,       // aliases: `short int` and `int short` and `signed` variationos thereof.
    TYPE_BASIC_INT,         // aliases: `signed int` and `int signed`.
    TYPE_BASIC_LONG,        // aliases: `long int`, `int long` and `signed` variations thereof.
    TYPE_BASIC_LONG_LONG,   // aliases: `long long int`, `int long long`, `long int long` and `signed` variations thereof.

    TYPE_BASIC_FLOAT,
    TYPE_BASIC_DOUBLE,
    TYPE_BASIC_LONG_DOUBLE, // ugh

    TYPE_BASIC_VOID,
    TYPE_BASIC_POINTER,

    TYPE_BASIC_STRUCT,
    TYPE_BASIC_ENUM,
    TYPE_BASIC_UNION,

    TYPE_BASIC_COUNT,
};

typedef enum Type_Modifier Type_Modifier;
enum Type_Modifier {
    TYPE_MOD_NONE,
    TYPE_MOD_SIGNED,
    TYPE_MOD_UNSIGNED,
    TYPE_MOD_COMPLEX,
    TYPE_MOD_COUNT,
};

#define BIT(N)  (1 << (N))

/**
 * @note
 *      Qualifiers can be nested, e.g. `const volatile int`.
 *      So each enumeration member (sans `TYPE_QUAL_COUNT`) actually represents
 *      a bit flag. Use the `BIT` macro to get them.
 */
typedef enum Type_Qualifier Type_Qualifier;
enum Type_Qualifier {
    TYPE_QUAL_CONST,
    TYPE_QUAL_VOLATILE,
    TYPE_QUAL_RESTRICT,
    TYPE_QUAL_COUNT,
};

typedef struct Type_Info Type_Info;
struct Type_Info {
    Type_Basic basic;
};

bool
type_parse_string(String text);
