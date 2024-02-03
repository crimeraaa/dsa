#ifndef TERRIBLE_GENERIC_ARRAY_H
#define TERRIBLE_GENERIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#define ga_typedef(type, ident) /* Empty to ensure compatibility with C. */
#else /* __cplusplus not defined */
#define nullptr NULL
/* C, I love you, but I don't want to type `struct` or `enum` everywhere... */
#define ga_typedef(type, ident) typedef type ident ident;
#endif /* __cplusplus */

#include <limits.h>     /* SCHAR_MIN */
#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* size_t, malloc, free */
#include <stdio.h>      /* perror */
#include <wchar.h>      /* wchar_t */

typedef unsigned char ga_byte;

enum ga_TypeLength {
    GA_TYPELENGTH_NONE,
    GA_TYPELENGTH_LONG,
    GA_TYPELENGTH_LLONG,
    GA_TYPELENGTH_SHORT,
    GA_TYPELENGTH_SSHORT,
    GA_TYPELENGTH_SIZE_T, // For `%zu` mainly.
    GA_TYPELENGTH_COUNT, // Total length of the lookup table.
}; ga_typedef(enum, ga_TypeLength)

/** 
 * We don't actually use this to avoid implicit conversion to `ga_TypeLength`.
 * Rather, this is just to gauge how many character type modifiers there are.
 */
enum ga_CharType {
    GA_CHARTYPE_NARROW,
    GA_CHARTYPE_WIDE,
    GA_CHARTYPE_COUNT
}; ga_typedef(enum, ga_CharType)

union ga_TypeValue {
    char c;
    int i;
    long li;
    long long lli;
    short hi;
    signed char hhi;
    unsigned int u;
    unsigned long lu;
    unsigned long long llu;
    unsigned short hu;
    unsigned char hhu;
    float f;
    double lf; // For our purposes we DON'T promote `float` to `double`.
    long double llf; // `%llf` is a GNU extension but it seems good to me.
    size_t zu;
    char *s;
    void *p; // Raw memory address or user-defined arbitrary struct/class.
}; ga_typedef(union, ga_TypeValue)

// For struct types which we'll can only use `void*` of, you NEED this!
typedef void ga_assignfn(void *dst, const void *src);

struct ga_TypeInfo {
    size_t size; // `sizeof` for this type.
    ga_TypeLength length;
    char spec; // One of: `c`, `i`, `u`, `f`, `s`, `p` ala `printf`.
    bool sign; // Is this a signed integer type?
    bool fund; // Is this a fundamental type?
    ga_assignfn *copy; // Only used for struct-types, must be provided by user.
}; ga_typedef(struct, ga_TypeInfo)

struct ga_array {
    const ga_TypeInfo *info;
    ga_byte *rawbytes; // Treat our array as just a giant byte-blokc.
    size_t count; // Number of items (not bytes!) currently in the buffer.  
    size_t capacity; // Total number of items (not bytes!) we've allocated for.
}; ga_typedef(struct, ga_array)

ga_array ga_init(size_t count, const ga_TypeInfo *info);
void ga_deinit(ga_array *self);
void *ga_assign(ga_array *self, void *dst, const void *src);
bool ga_push_back(ga_array *self, const void *src);
void *ga_retrieve(const ga_array *self, size_t index);

struct ga_fundtype_lookup {
    const ga_TypeInfo i[GA_TYPELENGTH_COUNT]; // Signed integer types.
    const ga_TypeInfo u[GA_TYPELENGTH_COUNT]; // Unsigned integer types.
    const ga_TypeInfo c[GA_CHARTYPE_COUNT]; // Narrow and wide character types.
    const ga_TypeInfo s[GA_CHARTYPE_COUNT]; // Narrow and wide character types.
}; ga_typedef(struct, ga_fundtype_lookup)

/**
 * Query the member that closely resembles your fundamental-type, then
 * use the appropriate `GA_*` enum to index into it.
 *
 * ```
 * [int | long | long long | short | signed char]: ga_fundtypes.i[GA_TYPELENGTH_*];
 * [unsigned (int|long|long long|short|char) | size_t]: ga_fundtypes.u[GA_TYPELENGTH_*];
 * [char | wchar_t]: ga_fundtypes.c[GA_CHARTYPE_(NARROW|WIDE)];
 * [char* | wchar_t*]: ga_funddtypes.s[GA_CHARTYPE_(NARROW|WIDE)];
 * ```
 * 
 * Indexing using number literals won't result in a compile error, but do so
 * at your own risk!!!
 */
extern const ga_fundtype_lookup ga_fundtypes;

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_ARRAY_H */
