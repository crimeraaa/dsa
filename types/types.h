/**
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp
 */
#pragma once

#include "../strings.h"
#include "../intern.h"

/**
 * @brief
 *      Tags for all the 'simple' types that are always present with C.
 *      These do not include aggregate types (e.g. structs, enums, unions),
 *      pointrs, nor typedefs.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L5
 */
typedef enum {
    CType_BasicKind_Invalid,
    CType_BasicKind_Bool,               // `_Bool` (C99-C17), `bool` (C23)

    // Integer Types
    CType_BasicKind_Char,               // `char` - distinct from signed/unsigned versions
    CType_BasicKind_Signed_Char,        // `signed char`, `char signed`
    CType_BasicKind_Short,              // `short`, `short int`, `int short` and signed versions thereof
    CType_BasicKind_Int,                // `int`, `signed int`, `int signed`, `signed`
    CType_BasicKind_Long,               // `long`, `long int`, `int long` and signed versions thereof
    CType_BasicKind_Long_Long,          // `long long`, `long long int`, `int long long`, `long int long` and signed versions thereof
    CType_BasicKind_Unsigned_Char,      // `unsigned char`
    CType_BasicKind_Unsigned_Short,     // `unsigned short`, `short unsigned`, `unsigned short int`, `unsigned int short`
    CType_BasicKind_Unsigned_Int,       // `unsigned int`, `int unsigned`, `unsigned`
    CType_BasicKind_Unsigned_Long,      // `unsigned long`, `long unsigned`, `unsigned long int`, `long int unsigned`
    CType_BasicKind_Unsigned_Long_Long, // `unsigned long long`, `long long unsigned`, `unsigned long long int`, `unsigned int long long`, ...

    // Floating-Point Types
    // NOTE: `_Complex` and `complex` are interchangeable.
    // NOTE: `_Imaginary` and `imaginary` are optional! Nobody really implements it.
    CType_BasicKind_Float,              // `float`
    CType_BasicKind_Double,             // `double`
    CType_BasicKind_Long_Double,        // `long double`, `double long`
    CType_BasicKind_Complex_Float,      // `complex float`, `float complex`
    CType_BasicKind_Complex_Double,     // `complex double`, `double complex`, `complex`
    CType_BasicKind_Complex_Long_Double,// `complex long double`, `complex double long`, `long complex double`, `long double complex`, `long complex`, `complex long`

    // Misc.
    CType_BasicKind_Void,               // `void`
    CType_BasicKind_Count,
} CType_BasicKind;

/**
 * @brief
 *      Tells us how to interpret a `CType`. Notice how pointers, structs, enums
 *      and unions are finally present.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L296
 */
typedef enum {
    CType_Kind_Invalid,
    CType_Kind_Basic,
    CType_Kind_Pointer,
    CType_Kind_Struct,
    CType_Kind_Enum,
    CType_Kind_Union,
    CType_Kind_Count,
} CType_Kind;

extern const String
ctype_kind_strings[CType_Kind_Count];

typedef struct CType CType;

#ifndef BIT
#define BIT(N) (1 << (N))
#endif // BIT

typedef enum {
    CType_BasicFlag_Bool        = BIT(0),   // `_Bool`, `bool`
    CType_BasicFlag_Integer     = BIT(1),   // `char`, `short`, `int`, `long`, `long long` and variants thereof.
    CType_BasicFlag_Float       = BIT(2),   // `float`, `double`, `long double` and variants thereof.
    CType_BasicFlag_Literal     = BIT(3),   // untyped, possible freely convertible: integer literals, float literals, etc.

    CType_BasicFlag_Signed      = BIT(4),   // `char` is not guaranteed to be signed.
    CType_BasicFlag_Unsigned    = BIT(5),   // `char` is not guaranteed to be unsigned.
    CType_BasicFlag_Complex     = BIT(6),   // `complex` variants of the types under `CType_BasicFlag_Float`.

    CType_BasicFlag_Numeric     = CType_BasicFlag_Integer | CType_BasicFlag_Signed | CType_BasicFlag_Unsigned | CType_BasicFlag_Float | CType_BasicFlag_Complex,
} CType_BasicFlag;

typedef enum {
    CType_QualifierFlag_Const    = BIT(0),  // Applicable to ALL types.
    CType_QualifierFlag_Volatile = BIT(1),  // Applicable to ALL types.
    CType_QualifierFlag_Restrict = BIT(2),  // Only valid for pointers.
} CType_QualifierFlag;

/**
 * @brief
 *      This represents concrete information about any type that exists in practice.
 *      This is separate from `CType` because is contains the qualifiers.
 *
 * @note
 *      It does allow for qualifiers like `const` and `volatile` because basic
 *      types, pointers, and aggregate types can be augmented with these.
 *
 *      Depending on the exact situation, they may be interchangeable with their
 *      base types or not.
 */
typedef struct {
    const CType         *type;       // Multiple `CType_Info` can refer to the same `CType`.
    CType_QualifierFlag  qualifiers; // Bit set of `CType_QualifierFlag`.
    bool                 is_owner;   // `type` is dynamically-allocated and we own it?
} CType_Info;

/**
 * @brief
 *      This represents the basic information for all the basic types. The basic
 *      types simply consists of all the integers, floating-points and `void`.
 *
 * @note
 *      Qualifiers like `const` or `volatile` are not present. They simply
 *      augment existing types rather than create new types on their own.
 *
 *      Also, qualifiers can be applied to pointers as well as aggregate types
 *      like `struct`, `union` and `enum`. So that is a problem for the more
 *      general `CType_Info`.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L117
 */
typedef struct {
    CType_BasicKind kind;
    CType_BasicFlag flags;  // Bitset of `CType_BasicFlag`.
    String          name;
} CType_Basic;

/**
 * @brief
 *      This represents information about any given pointer type. Notice how
 *      we point to a `CType_Info` and not a `CType` because the type we point to
 *      can be qualified.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L218
 */
typedef struct {
    const CType_Info   *pointee;
    CType_QualifierFlag qualifiers;
} CType_Pointer;

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L131
typedef struct {
    /* TODO */ int i;
} CType_Struct;


// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L244
typedef struct {
    /* TODO */ int i;
} CType_Enum;

// https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L131
typedef struct {
    /* TODO */ int i;
} CType_Union;

/**
 * @brief
 *      This represents basic data about *any* type, including pointers and
 *      aggregate types or user-defined types.
 *
 * @link
 *      https://github.com/odin-lang/Odin/blob/master/src/types.cpp#L321
 */
struct CType {
    CType_Kind kind;
    union {
        CType_Basic   basic;
        CType_Pointer pointer;
        CType_Struct  struct_;
        CType_Enum    enum_;
        CType_Union   union_;
    };
};

extern const CType
ctype_basic_types[CType_BasicKind_Count];

typedef struct {
    CType_Info *info; // Each is dynamically allocated so they can be shared.
} CType_Entry;

/**
 * @note
 *      The indexes, 0 up to `CType_BasicKind_Count - 1`, must be of type
 *      `CType_BasicKind`. They must be unqualified.
 */
typedef struct {
    Allocator    allocator;
    CType_Entry *entries;
    size_t       len;
    size_t       cap;
} CType_Table;

Allocator_Error
ctype_table_init(CType_Table *table, Allocator allocator);

void
ctype_table_destroy(CType_Table *table);

const CType_Info *
ctype_table_get_basic_unqual(CType_Table *table, CType_BasicKind kind);

const CType_Info *
ctype_table_get_basic_qual(CType_Table *table, CType_BasicKind kind, CType_QualifierFlag qualifiers);

const CType_Info *
ctype_table_add_basic_qual(CType_Table *table, CType_BasicKind kind, CType_QualifierFlag qualifiers);

#undef BIT
