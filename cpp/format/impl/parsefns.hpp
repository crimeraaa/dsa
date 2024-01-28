#pragma once

/**
 * @file        parsefns.hpp
 * 
 * @author      crimeraaa
 * 
 * @date        28 January 2024
 * 
 * @brief       Forward declaration and documentation of internal implementation 
 *              functions used in `parse.cpp`.
 * 
 * @warning     These internal functions are not designed to be C-compatible.
 */
#pragma once

#include <cstdarg>
#include <cstdio>

#include "parsetypes.hpp"

/**
 * @brief   Does the heavy lifting of parsing a single instance of a `%`.
 *          Lots of work goes into this, see C's printf spec.
 *
 * @param   stream      Where the output is heading.
 * @param   next        Pointer to character right after `spec`.
 * @param   args        Variadic argument list on the stack. Pop as we go!
 * @param   spec        Character directly to the right of `'%'`.
 * 
 * @return  Pointer to the next non-format specifier character in the format string.
 */
const char *parse_fmt(std::FILE *stream, const char *next, std::va_list args, char spec);

/**
 * @brief   Check the current specifier for its modifiers.
 * 
 * @param   next    Pointer to 1 past the character directly right of '%'.
 * @param   spec    Current character being parsed, not the same as `*next`.
 * 
 * @return  FmtSpec struct containing information we can use to pretty print.
 */
FmtSpec parse_mod(const char *next, char spec);

/**
 * @brief   Calls the appropriate template instantiation of `write_integer`
 *          for one of: `int`, `long` and `long long`.
 *          
 * @note    `char` and `short` are automatically promoted to `int` in
 *          variadic arguments in C.
 */
void write_signed(std::FILE *stream, std::va_list args, FmtMod mod, int base = 10);

/**
 * @brief   Calls the appropriate template instantiation of `write_integer`
 *          for one of: `unsigned int`, `unsigned long` and `unsigned long long`.
 * 
 * @note    `unsigned char` and `unsigned short` are automatically promoted to
 *          `unsinged int` in variadic arguments in C.
 */
void write_unsigned(std::FILE *stream, std::va_list args, FmtMod mod, int base = 10);
