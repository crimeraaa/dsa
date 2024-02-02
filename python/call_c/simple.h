#ifndef SIMPLE_DUMPBYTES_H
#define SIMPLE_DUMPBYTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h> /* CHAR_BIT */
#include <stdio.h>  /* printf, putchar */
#include <stdlib.h> /* size_t */
#include <string.h> /* strlen */
#include <wchar.h>  /* wchar_t, wcslen */

/* All results of `sizeof` are in terms of `char`. */
typedef unsigned char ga_byte;

/* Write individual buffer of `byte` to stack-allocated `char` array `buffer`. */
char *byte_to_bits(ga_byte byte, char *buffer);

/**
 * Write the individual bits of each byte pointed to by `p_memory`.
 * Best used with stack-allocated variables that you just pass the address of!
 */
void dump_bytes(const void *p_memory, size_t n_sizeof);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* SIMPLE_DUMPBYTES_H */
