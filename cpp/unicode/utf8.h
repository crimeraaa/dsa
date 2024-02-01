#ifndef SIMPLE_UTF8_ENCODING_H
#define SIMPLE_UTF8_ENCODING_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <limits.h> /* MB_LEN_MAX */
#include <stdlib.h> /* size_t, malloc, free */
#include <stdio.h>  /* EOF, fputc, fputs, perror */
#include <string.h> /* strlen */
#include <wchar.h>  /* wchar_t, fputwc, fputws, wcslen, mbsrtowcs, wcsrtombs */

/**
 * @brief   Writes the wide character `wc` to narrow oriented `stream`.
 *          Designed to be compatible with C. Assumes a UTF-8 locale.
 *          
 * @note    No heap allocations are made here as we know we can use the
 *          `MB_LEN_MAX` macro to determine the maximum possible size of the OS
 *          encoding, for this particular system.
 *          
 * @return  Result of `fputs` or `EOF`.
 */
int narrow_fputwc(int wc, FILE *stream);

/**
 * @brief   Writes the wide character string `ws` to narrow oriented `stream`.
 *          Designed to be compatible with C. Assumes a UTF-8 locale.
 *          
 * @note    In order not to make any assumptions about string length, we have to
 *          use heap allocations to ensure we can fit the number of bytes that
 *          `ws`, encoded in UTF-8, would take up.
 *          
 * @return  Result of `fputs` or `EOF`.
 */
int narrow_fputws(const wchar_t *ws, FILE *stream);

/**
 * @brief   Writes the character `c`, promoted to `int`, to wide oriented `stream`.
 *          Designed to be compatible with C. Assumes a UTF-8 locale.
 *          
 * @return  Result of `fputwc` (cast to `int` for consistency) or `EOF`.
 */
int wide_fputc(int c, FILE *stream);

/**
 * @brief   Wries the nul-terminated character string `s` to wide oriented `stream`.
 *          Designed to be compatible with C. Assumes a UTF-8 locale.
 *          
 * @note    In order to not make assumptions about string length, we have to use
 *          heap allocations for the internal `char` buffer.
 *          
 * @return  Result of `fputws` or `-1`.
 */
int wide_fputs(const char *s, FILE *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus (extern "C" #1) */
#endif /* SIMPLE_UTF8_ENCODING_H */
