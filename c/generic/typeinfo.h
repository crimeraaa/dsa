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

typedef void *ti_initfn(void *dst);
typedef void *ti_copyfn(void *dst, const void *src);
typedef void *ti_movefn(void *dst, void *src);
typedef void ti_deinitfn(void *dst);

typedef struct ti_typefns {
    ti_initfn *init; // C++ style Constructor. What's this type's defaults?
    ti_copyfn *copy; // C++ style Copy-constructor.
    ti_movefn *move; // C++ style Move-constructor. 
    ti_deinitfn *deinit; // C++ style Destructor. How do we clean up this type?
} ti_typefns;

typedef enum ti_typelength {
    TI_LENGTH_NONE,
    TI_LENGTH_LONG,   // `_LONG` right here so that char and wchar are nearby.
    TI_LENGTH_LLONG,
    TI_LENGTH_SHORT,
    TI_LENGTH_SSHORT,
    TI_LENGTH_SIZE_T, // For `%zu` mainly.
    TI_LENGTH_COUNT,  // Total length of the lookup table.
} ti_typelength;

typedef struct ti_typeinfo {
    size_t size; // `sizeof` for this type.
    const ti_typefns *fnlist; // Basic functions for manipulating our datatype.
    ti_typelength length; // What's our length modifier? e.g. `long` or `short`.
    char spec; // One of: `c`, `i`, `u`, `f`, `s`, `p` ala `printf`.
    bool is_signed; // Is this a signed integer type?
    bool is_fundamental;
} ti_typeinfo; 

typedef struct ti_lookup {
    const ti_typeinfo i[TI_LENGTH_COUNT]; // Signed integer types.
    const ti_typeinfo u[TI_LENGTH_COUNT]; // Unsigned integer types.
    const ti_typeinfo c[TI_LENGTH_COUNT]; // Narrow and wide character types.
    const ti_typeinfo s[TI_LENGTH_COUNT]; // Narrow and wide character types.
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

/**
 * @brief   Helper function to make querying `ti_fundtypes` easier.
 * 
 * @param   spec    One of `i`, `u`, `c`, `s` or `p`. Think `printf` specifiers.
 * @param   len     A `TI_LENGTH_*` enum value.
 * 
 * @note    For `p`, you can pass any `TI_LENGTH_*` macro as there is only 1 entry
 *          for it.
 */
const ti_typeinfo *ti_query(char spec, ti_typelength len);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_TYPEINFO_H */
