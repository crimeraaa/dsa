/**
 * @file        parse.cpp
 *
 * @author      crimeraaa
 *
 * @date        27 January 2024
 * 
 * @brief       Actual implementation of `parse.h`'s exposed functions.
 */
#include <cassert>
#include <cstdarg> /* va_list and accompaniying functions */
#include <cstddef>
#include <cstdio> /* Too lazy to reimplement fputc, fputs and friends */
#include <cwchar> /* std::fputwc, std::fputws */

#include "parse.h"
#include "impl/parsetypes.hpp"
#include "impl/parsefns.hpp"
#include "impl/write_integer.impl.hpp"

void write_stream(std::FILE *stream, const char *fmts, ...)
{
    std::va_list args;
    va_start(args, fmts); // args now points to first vararg, whatever it may be
    return write_va_list(stream, fmts, args);
}

void write_va_list(std::FILE *stream, const char *fmts, std::va_list args)
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
    FmtSpec what = parse_mod(next, ch);
    switch (what.tag)
    {
        case '%': {
            break;
        }
        case 'c': {
            // Both `char` and `wchar_t` are promoted to `int`.
            if (what.mod == FmtMod::is_long) {
                std::fputwc(va_arg(args, int), stream);
            } else {
                std::fputc(va_arg(args, int), stream);
            }
            break;
        }
        case 'd': // fall-through
        case 'i': {
            write_signed(stream, args, what.mod);
            break;
        }
        case 'u': {
            write_unsigned(stream, args, what.mod);
            break;
        }
        case 'f': {
            // Floats are promoted to double automatically.
            double f = va_arg(args, double);
            (void)f; // ignore for now, idk what to do
            break;
        }
        case 's': {
            if (what.mod == FmtMod::is_long) {
                std::fputws(va_arg(args, wchar_t*), stream);
            } else {
                std::fputs(va_arg(args, char*), stream);
            }
            break;
        }
        case 'p': {
            void *p = va_arg(args, void*);
            (void)p;
            break;
        }
        default: {
            break;
        }
    }
    return what.nextptr;
}

FmtSpec parse_mod(const char *next, char ch)
{
    // For now, use `nextptr` to peek ahead of `ch` in the format string.
    FmtSpec what;
    what.nextptr = next;
    what.mod = FmtMod::is_none;
    what.tag = ch; // If no modifiers found this will stay as-is.
    switch (ch)
    {
        case 'l': {
            what.mod = FmtMod::is_long; 
            if (*what.nextptr == 'l') {
                what.mod = FmtMod::is_long_long;
                what.nextptr++;
            }
            // Remember postfix increment returns the previous value
            what.tag = *what.nextptr++; 
            break;
        }        
        case 'h': {
            what.mod = FmtMod::is_short;
            if (*what.nextptr == 'h') {
                what.mod = FmtMod::is_short_short;
                what.nextptr++;
            }
            what.tag = *what.nextptr++;
            break;
        }
        default: {
            break;
        }
    }
    return what;
}

void write_signed(std::FILE *stream, std::va_list args, FmtMod mod, int base)
{
    switch (mod) 
    {
        case FmtMod::is_long: {
            write_integer(stream, va_arg(args, long), base);
            break;
        }
        case FmtMod::is_long_long: {
            write_integer(stream, va_arg(args, long long), base);
            break;
        }
        case FmtMod::is_short: // fall-through
        case FmtMod::is_short_short: // fall-through
        case FmtMod::is_none: {
            write_integer(stream, va_arg(args, int), base);
            break;
        }
    }
}

void write_unsigned(std::FILE *stream, std::va_list args, FmtMod mod, int base)
{
    switch (mod)
    {
        case FmtMod::is_long: {
            write_integer(stream, va_arg(args, unsigned long), base);
            break;
        }
        case FmtMod::is_long_long: {
            write_integer(stream, va_arg(args, unsigned long long), base);
            break;
        }
        case FmtMod::is_short: // fall-through
        case FmtMod::is_short_short: // fall-through
        case FmtMod::is_none: {
            write_integer(stream, va_arg(args, unsigned int), base);
            break;
        }
    }   
}
