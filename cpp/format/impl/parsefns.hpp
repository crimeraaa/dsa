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
 * @return  Pointer to the last format specifier character for this item.
 */
const char *parse_fmt(std::FILE *stream, const char *next, std::va_list args, char spec);

/**
 * @brief   Check the current specifier for its length modifiers.
 * 
 * @param   next    Pointer to 1 past the character directly right of '%'.
 * @param   spec    Current character being parsed, not the same as `*next`.
 * 
 * @return  `FmtParse` struct containing information we can use to pretty print.
 */
FmtParse parse_len(const char *next, char spec);

/**
 * @brief   Chooses which overload of `print_int_body` to use.
 */
void print_number_to(std::FILE *stream, std::va_list args, const FmtParse &what);


/** 
 * Both `char` and `wchar_t` are promoted to `int`.
 * However their signedness is unspecified by the C standard.
 * 
 * @bug     It seems I can't mix `fputwc` and `fputc` in the same stream.
 *          I wonder what `printf` is doing to make it work then?
 */
void print_char_to(std::FILE *stream, std::va_list args, const FmtParse &what);
/**
 * See notes for `print_char_to`. I wonder what `printf` is doing to make
 * mixing of `"%s"` and `"%ls"` in the same stream work.
 */
void print_string_to(std::FILE *stream, std::va_list args, const FmtParse &what);
