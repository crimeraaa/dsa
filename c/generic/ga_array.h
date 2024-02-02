#ifndef TERRIBLE_GENERIC_ARRAY_H
#define TERRIBLE_GENERIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#else
#define nullptr NULL
#endif /* __cplusplus */

#include <stdbool.h>    /* bool */
#include <stdlib.h>     /* size_t, malloc, free */
#include <stdio.h>      /* perror */
#include <wchar.h>      /* wchar_t */

typedef unsigned char ga_byte;

enum ga_TypeLength {
    GA_TYPE_LENGTH_NONE,
    GA_TYPE_LENGTH_LONG,
    GA_TYPE_LENGTH_LLONG,
    GA_TYPE_LENGTH_SHORT,
    GA_TYPE_LENGTH_SSHORT,
    GA_TYPE_LENGTH_SIZE_T, // For `%zu` mainly.
};

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
};

typedef struct ga_TypeInfo {
    union ga_TypeValue type;
    enum ga_TypeLength length;
    char spec;
    bool sign;
} ga_TypeInfo;

typedef struct ga_array {
    ga_byte *rawbytes;
    size_t count; // Number of items (not bytes!) currently in the buffer.
    size_t capacity; // Total number of items (not bytes!) we've allocated for.
    size_t size; // `sizeof` of the desired element.
    ga_TypeInfo info;
} ga_array;

ga_array ga_init(size_t count, size_t size);
void ga_deinit(ga_array *self);
void *ga_assign(ga_array *self, void *dst, const void *src);
bool ga_push_back(ga_array *self, const void *src);
void *ga_retrieve(const ga_array *self, size_t index);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* TERRIBLE_GENERIC_ARRAY_H */
