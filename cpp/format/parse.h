/**
 * @file        parse.h
 *
 * @author      crimeraaa
 *
 * @date        27 January 2024
 *
 * @brief       This is the user-facing header file designed to be included in
 *              both C and C++ source files.
 *
 * @note        Although this is compatible for both C and C++, do note that
 *              compiling the object files must be done with a C++ compiler.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#ifdef __GNUC__
/**
 * @brief   Using the `format` attribute (a GCC extension) we can statically
 *          check calls to this function for correctness.
 *          - https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Function-Attributes.html
 *
 * @param   fmts    1-based index of the last non-variadic argument.
 * @param   args    1-based index of the variadic arguments themselves.
 *
 * @note    When using a `va_list`, specify `args` as 0.
 */
#define check_format(fmts, args) __attribute__ ((format (printf, fmts, args)))
#else /* __GNUC__ not defined, so leave it as an empty macro. */
#define check_format(fmts, args)
#endif /* __GNUC__ */

/** 
 * @brief   My personal implementation of C-style `printf`. For fun!
 *          Writes to C `stdout`, assuming a narrow-stream orientation.
 *
 * @param   fmts        Format string with 0/more format specifiers.
 * @param   ...         Arguments to the format string.
 *
 * @note    This is (should be, anyway) C compatible!
 */
void print_format(const char *fmts, ...) check_format(1, 2);

/**
 * @brief   My personal implementation of the C standard library's `vfprintf`.
 * 
 * @note    Heavily inspired by GNU libc's implementation!
 */
void print_args_to(FILE *stream, const char *fmts, va_list args) check_format(2, 0);

/**
 * @brief   Print a wide character to a narrow stream. Useful for UTF-8.
 * 
 *          Converts the wide character `arg` to a `char` buffer then writes
 *          said buffer to `stream`.
 * 
 * @return  Number of raw `char`'s written or `EOF`.
 */
int print_mbchar_to(int arg, FILE *stream);

/**
 * @brief   Print a wide character string to a narrow stream. Useful for UTF-8.
 * 
 * @note    Allocates memory for an internal buffer in order to convert `arg`.
 *          
 * @return  Number of raw `char`'s written or `EOF`.
 */
int print_mbstring_to(const wchar_t *arg, FILE *stream);

#ifdef __cplusplus
}
#endif /* extern C */
