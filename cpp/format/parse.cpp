/**
 * @file        parse.cpp
 *
 * @author      crimeraaa
 *
 * @date        27 January 2024
 * 
 * @brief       Actual implementation of `parse.h`'s exposed functions.
 */
#include <cstdarg> /* va_list and accompaniying functions */
#include <cstddef> /* std::size_t */
#include <cstdio> /* std::FILE, std::fputc, std::fputs */
#include <cwchar> /* std::fputwc, std::fputws */
#include <type_traits> /* std::is_signed */

#include "parse.h"
#include "impl/parsetypes.hpp"
#include "impl/parsefns.hpp"
#include "impl/printfns.impl.hpp"

void print_format(const char *fmts, ...)
{
    std::va_list args;
    va_start(args, fmts); // args now points to first vararg, whatever it may be
    return print_args_to(stdout, fmts, args);
}

void print_args_to(std::FILE *stream, const char *fmts, std::va_list args)
{
    const char *ptr = fmts; // Keep track of current character in format string.
    bool is_fmtspec = false;
    char ch;
    // ptr itself is incremented, but postfix returns the original value
    while ((ch = *ptr++) != '\0') {
        if (ch == '%') {
            is_fmtspec = true;
            continue; // Proceed to next token
        }
        if (!is_fmtspec) {
            std::fputc(ch, stream);
        } else {
            ptr = parse_fmt(stream, ptr, args, ch);
            is_fmtspec = false;
        }
    }
    va_end(args);
}

const char *parse_fmt(std::FILE *stream, const char *next, std::va_list args, char ch)
{
    FmtParse what = parse_len(next, ch);
    switch (what.spec)
    {
        case '%': {
            std::fputc('%', stream);
            break;
        }
        case 'c': {
            print_char_to(stream, args, what);
            break;
        }
        case 'd': // fall-through
        case 'i': {
            what.base = 10;
            what.is_signed = true;
            print_number_to(stream, args, what);
            break;
        }
        case 'u': {
            what.base = 10;
            what.is_signed = false;
            print_number_to(stream, args, what);
            break;
        }
        case 'f': {
            // Floats are promoted to double automatically.
            double f = va_arg(args, double);
            (void)f; // ignore for now, idk what to do
            break;
        }
        case 's': {
            print_string_to(stream, args, what);
            break;
        }
        case 'p': {
            void *p = va_arg(args, void*);
            (void)p;
            break;
        }
        case 'x': {
            what.base = 16;
            what.is_signed = false;
            print_number_to(stream, args, what);
            break;
        }
        default: {
            break;
        }
    }
    return what.endptr;
}

FmtParse parse_len(const char *next, char ch)
{
    // For now, use `endptr` to peek ahead of `ch` in the format string.
    FmtParse what;
    what.endptr = next;
    what.len = FmtLen::is_none;
    what.spec = ch; // If no modifiers found this will stay as-is.
    switch (ch)
    {
        case 'l': {
            what.len = FmtLen::is_long; 
            if (*what.endptr == 'l') {
                what.len = FmtLen::is_long_long;
                what.endptr++;
            }
            // Remember postfix increment returns the previous value.
            what.spec = *what.endptr++; 
            break;
        }        
        case 'h': {
            what.len = FmtLen::is_short;
            if (*what.endptr == 'h') {
                what.len = FmtLen::is_short_short;
                what.endptr++;
            }
            what.spec = *what.endptr++;
            break;
        }
        default: {
            break;
        }
    }
    return what;
}

void print_number_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    switch (what.len)
    {
        case FmtLen::is_long:
            print_int_to<long>(stream, args, what.base, what.is_signed);
            break;
        case FmtLen::is_long_long:
            print_int_to<long long>(stream, args, what.base, what.is_signed);
            break;
        case FmtLen::is_short: // fall-through
        case FmtLen::is_short_short: // fall-through
        case FmtLen::is_none:
            print_int_to<int>(stream, args, what.base, what.is_signed);
            break;
    }    
}

void print_char_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    if (what.len == FmtLen::is_long) {
        std::fputwc(va_arg(args, int), stream);
    } else {
        std::fputc(va_arg(args, int), stream);
    }
}

void print_string_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    if (what.len == FmtLen::is_long) {
        std::fputws(va_arg(args, const wchar_t*), stream);
    } else {
        std::fputs(va_arg(args, const char*), stream);
    }
}
