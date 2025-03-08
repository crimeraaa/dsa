/**
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp
 */
#pragma once

#include "../strings.h"

/**
 * @brief
 *      Tags for all the 'simple' types that are always present with C.
 *      These do not include aggregate types (e.g. structs, enums, unions),
 *      pointrs, nor typedefs.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L5
 */
enum Type_BasicKind {
    Type_BasicKind_Invalid,
    Type_BasicKind_Bool,                // `_Bool` (C99-C17), `bool` (C23)

    // Integer Types
    Type_BasicKind_Char,                // `char` - distinct from signed/unsigned versions
    Type_BasicKind_Signed_Char,         // `signed char`, `char signed`
    Type_BasicKind_Short,               // `short`, `short int`, `int short` and signed versions thereof
    Type_BasicKind_Int,                 // `int`, `signed int`, `int signed`, `signed`
    Type_BasicKind_Long,                // `long`, `long int`, `int long` and signed versions thereof
    Type_BasicKind_Long_Long,           // `long long`, `long long int`, `int long long`, `long int long` and signed versions thereof
    Type_BasicKind_Unsigned_Char,       // `unsigned char`
    Type_BasicKind_Unsigned_Short,      // `unsigned short`, `short unsigned`, `unsigned short int`, `unsigned int short`
    Type_BasicKind_Unsigned_Int,        // `unsigned int`, `int unsigned`, `unsigned`
    Type_BasicKind_Unsigned_Long,       // `unsigned long`, `long unsigned`, `unsigned long int`, `long int unsigned`
    Type_BasicKind_Unsigned_Long_Long,  // `unsigned long long`, `long long unsigned`, `unsigned long long int`, `unsigned int long long`, ...

    // Floating-Point Types
    // NOTE: `_Complex` and `complex` are interchangeable.
    // NOTE: `_Imaginary` and `imaginary` are optional! Nobody really implements it.
    Type_BasicKind_Float,               // `float`
    Type_BasicKind_Double,              // `double`
    Type_BasicKind_Long_Double,         // `long double`, `double long`
    Type_BasicKind_Complex_Float,       // `complex float`, `float complex`
    Type_BasicKind_Complex_Double,      // `complex double`, `double complex`, `complex`
    Type_BasicKind_Complex_Long_Double, // `complex long double`, `complex double long`, `long complex double`, `long double complex`, `long complex`, `complex long`

    // Misc.
    Type_BasicKind_Void,                // `void`
    Type_BasicKind_Count,
};
typedef enum Type_BasicKind Type_BasicKind;

/**
 * @brief
 *      Tells us how to interpret a `Type`. Notice how pointers, structs, enums
 *      and unions are finally present.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L296
 */
enum Type_Kind {
    Type_Kind_Invalid,
    Type_Kind_Basic,
    Type_Kind_Pointer,
    Type_Kind_Struct,
    Type_Kind_Enum,
    Type_Kind_Union,
    Type_Kind_Count,
};
typedef enum Type_Kind Type_Kind;

extern const String
TYPE_KIND_STRINGS[Type_Kind_Count];

typedef struct Type Type;

#ifndef BIT
#define BIT(N) (1 << (N))
#endif // BIT

enum Type_BasicFlag {
    Type_BasicFlag_Bool        = BIT(0), // `_Bool`, `bool`
    Type_BasicFlag_Integer     = BIT(1), // `char`, `short`, `int`, `long`, `long long` and variants thereof.
    Type_BasicFlag_Float       = BIT(2), // `float`, `double`, `long double` and variants thereof.
    Type_BasicFlag_Literal     = BIT(3), // untyped, possible freely convertible: integer literals, float literals, etc.

    Type_BasicFlag_Signed      = Type_BasicFlag_Integer | BIT(4), // `char` is not guaranteed to be signed.
    Type_BasicFlag_Unsigned    = Type_BasicFlag_Integer | BIT(5), // `char` is not guaranteed to be unsigned.
    Type_BasicFlag_Complex     = Type_BasicFlag_Float   | BIT(6), // `complex` variants of the types under `Type_BasicFlag_Float`.
    Type_BasicFlag_Numeric     = Type_BasicFlag_Integer | Type_BasicFlag_Signed | Type_BasicFlag_Unsigned | Type_BasicFlag_Float | Type_BasicFlag_Complex,
};
typedef enum Type_BasicFlag Type_BasicFlag;

#undef BIT

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L117
typedef struct Type_Basic Type_Basic;
struct Type_Basic {
    Type_BasicKind kind;
    uint32_t       flags;  // See `Type_BasicFlag`.
    String         name;
};

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L218
typedef struct Type_Pointer Type_Pointer;
struct Type_Pointer {
    Type *pointee;
};

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L131
typedef struct Type_Struct Type_Struct;
struct Type_Struct {/* TODO */ int i;};

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L244
typedef struct Type_Enum Type_Enum;
struct Type_Enum {/* TODO */ int i;};

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L131
typedef struct Type_Union Type_Union;
struct Type_Union {/* TODO */ int i;};

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L321
struct Type {
    Type_Kind kind;
    union {
        Type_Basic   basic;
        Type_Pointer pointer;
        Type_Struct  struct_;
        Type_Enum    enum_;
        Type_Union   union_;
    };
};

extern const Type
TYPE_BASIC_TYPES[Type_BasicKind_Count];

#ifdef TYPES_IMPLEMENTATION

// NOTE: Ensure the order matches `Type_Kind`!
const String
TYPE_KIND_STRINGS[Type_Kind_Count] = {
    string_literal("Type_Kind_Invalid"),
    string_literal("Type_Kind_Basic"),
    string_literal("Type_Kind_Pointer"),
    string_literal("Type_Kind_Struct"),
    string_literal("Type_Kind_Enum"),
    string_literal("Type_Kind_Union"),
};

// NOTE: Ensure the order matches `Type_BasicKind`!
// TODO: Do we care about size?
const Type
TYPE_BASIC_TYPES[Type_BasicKind_Count] = {
    // Type_Kind,       Type_Basic{.kind,                   .flags,                     .name}
    {Type_Kind_Invalid, {{Type_BasicKind_Invalid,            0,                          string_literal("<invalid>")}}},

    // Booleans, Characters
    {Type_Kind_Basic,   {{Type_BasicKind_Bool,               Type_BasicFlag_Bool,        string_literal("bool")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Char,               Type_BasicFlag_Integer,     string_literal("char")}}},

    // Signed Integers
    {Type_Kind_Basic,   {{Type_BasicKind_Signed_Char,        Type_BasicFlag_Signed,      string_literal("signed char")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Short,              Type_BasicFlag_Signed,      string_literal("short")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Int,                Type_BasicFlag_Signed,      string_literal("int")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Long,               Type_BasicFlag_Signed,      string_literal("long")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Long_Long,          Type_BasicFlag_Signed,      string_literal("long long")}}},

    // Unsigned Integers
    {Type_Kind_Basic,   {{Type_BasicKind_Unsigned_Char,      Type_BasicFlag_Unsigned,    string_literal("unsigned char")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Unsigned_Short,     Type_BasicFlag_Unsigned,    string_literal("unsigned short")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Unsigned_Int,       Type_BasicFlag_Unsigned,    string_literal("unsigned int")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Unsigned_Long,      Type_BasicFlag_Unsigned,    string_literal("unsigned long")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Unsigned_Long_Long, Type_BasicFlag_Unsigned,    string_literal("unsigned long long")}}},

    // Floating-Point Types
    {Type_Kind_Basic,   {{Type_BasicKind_Float,              Type_BasicFlag_Float,       string_literal("float")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Double,             Type_BasicFlag_Float,       string_literal("double")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Long_Double,        Type_BasicFlag_Float,       string_literal("long double")}}},

    // Complex Types
    {Type_Kind_Basic,   {{Type_BasicKind_Float,              Type_BasicFlag_Complex,     string_literal("complex float")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Double,             Type_BasicFlag_Complex,     string_literal("complex double")}}},
    {Type_Kind_Basic,   {{Type_BasicKind_Long_Double,        Type_BasicFlag_Complex,     string_literal("complex long double")}}},

    // Misc. Types
    {Type_Kind_Basic,   {{Type_BasicKind_Void,               0,                          string_literal("void")}}},
};

#endif // TYPES_IMPLEMENTATION
