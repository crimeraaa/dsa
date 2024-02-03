#ifndef TERRIBLE_GENERIC_TYPEINFO_H
#define TERRIBLE_GENERIC_TYPEINFO_H

#ifdef __cplusplus
extern "C" {
#else 
#define nullptr NULL
#endif
    
#include <limits.h>     /* CHAR_MIN, SCHAR_MIN */
#include <stdbool.h>    /* bool */
#include <stddef.h>     /* size_t */
#include <wchar.h>      /* wchar_t, WCHAR_MIN */

typedef void ti_copyfn(void *dst, const void *src);
typedef void ti_movefn(void *dst, void *src);

typedef enum ti_typelength {
    TI_LENGTH_NONE,
    TI_LENGTH_LONG,   // `_LONG` right here so that char and wchar are nearby.
    TI_LENGTH_LLONG,
    TI_LENGTH_SHORT,
    TI_LENGTH_SSHORT,
    TI_LENGTH_SIZE_T, // For `%zu` mainly.
    TI_LENGTH_COUNT,  // Total length of the lookup table.
} ti_typelength;

/** 
 * We don't actually use this to avoid implicit conversion to `ti_typelength`.
 * Rather, this is just to gauge how many character type modifiers there are.
 */
typedef enum ti_chartype {
    TI_CHAR_NARROW,
    TI_CHAR_WIDE,
    TI_CHAR_COUNT
} ti_chartype;

typedef struct ti_typeinfo {
    size_t size; // `sizeof` for this type.
    ti_typelength length;
    char spec; // One of: `c`, `i`, `u`, `f`, `s`, `p` ala `printf`.
    bool sign; // Is this a signed integer type?
    bool fund; // Is this a fundamental type?
} ti_typeinfo; 

typedef struct ti_lookup {
    const ti_typeinfo i[TI_LENGTH_COUNT]; // Signed integer types.
    const ti_typeinfo u[TI_LENGTH_COUNT]; // Unsigned integer types.
    const ti_typeinfo c[TI_CHAR_COUNT];   // Narrow and wide character types.
    const ti_typeinfo s[TI_CHAR_COUNT];   // Narrow and wide character types.
    const ti_typeinfo p; // Void pointers, opaque pointers, etc.
} ti_lookup;

/**
 * Query the member that closely resembles your fundamental-type, then
 * use the appropriate `TI_*` enum to index into it.
 * 
 * Indexing using number literals won't result in a compile error, but do so
 * at your own risk!!!
 */
extern const ti_lookup ti_fundtypes;

const ti_typeinfo *ti_query(char spec, ti_typelength len);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_TYPEINFO_H */
