/**
 * @file        parse.cpp
 *
 * @author      crimeraaa
 *
 * @date        27 January 2024
 *
 * @brief       Actual implementation of `parse.h`'s exposed functions.
 *              Do note that I'm trying to limit dependencies on `libstdc++`,
 *              so I opt not to call operators `new` and `delete` here.
 * 
 *              That *should* allow `gcc` to link to the resulting object file
 *              just fine.
 */
#include <climits> /* MB_LEN_MAX */
#include <cstdarg> /* va_list and accompaniying functions */
#include <cstddef> /* std::size_t */
#include <cstdio> /* std::FILE, std::fputc, std::fputs */
#include <cstring> /* std::memset */
#include <cwchar> /* std::fputwc, std::fputws */
#include <memory> /* std::allocator, std::allocator_traits */
#include <type_traits> /* std::is_signed */

#include "parse.h"
#include "impl/parsetypes.hpp"
#include "impl/parsefns.hpp"
#include "impl/printfns.tpl.hpp"

/* Error code returned by `wctomb, wcrtomb, wcstombs, wcsrtombs`. */
constexpr size_t WC_TO_MB_ERROR = static_cast<size_t>(-1);

void print_format(const char *fmts, ...)
{
    std::va_list args;
    va_start(args, fmts); // args now points to first vararg, whatever it may be
    print_args_to(stdout, fmts, args);
    va_end(args);
}

void print_args_to(std::FILE *stream, const char *fmts, std::va_list args)
{
    const char *ptr = fmts; // Keep track of current character in format string.
    bool is_spec = false;
    char ch;
    // ptr itself is incremented, but postfix returns the original value
    while ((ch = *ptr++) != '\0') {
        if (ch == '%') {
            is_spec = true;
            continue; // Proceed to next token
        }
        if (!is_spec) {
            std::fputc(ch, stream);
        } else {
            ptr = parse_fmt(stream, ptr, args, ch);
            is_spec = false;
        }
    }
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

int print_number_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    int base = what.base;
    bool is_signed = what.is_signed;
    switch (what.len)
    {
        case FmtLen::is_long:
            return print_int_to<long>(stream, args, base, is_signed);
        case FmtLen::is_long_long:
            return print_int_to<long long>(stream, args, base, is_signed);
        case FmtLen::is_short: // fall-through
        case FmtLen::is_short_short: // fall-through
        case FmtLen::is_none:
            return print_int_to<int>(stream, args, base, is_signed);
    }
    return EOF; // Compiler warning, seems to not recognize the limited cases...
}

int print_char_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    return (what.len == FmtLen::is_long)
        ? print_mbchar_to(va_arg(args, int), stream)
        : std::fputc(va_arg(args, int), stream);
}

int print_string_to(std::FILE *stream, std::va_list args, const FmtParse &what)
{
    return (what.len == FmtLen::is_long)
        ? print_mbstring_to(va_arg(args, const wchar_t *), stream)
        : std::fputs(va_arg(args, const char *), stream);
}

int print_mbchar_to(int arg, std::FILE *stream)
{
    char mbchar[MB_LEN_MAX] = {0}; // valid for any of this system's encodings
    mbstate_t mbstate; // shift state
    std::memset(&mbstate, 0, sizeof(mbstate));
    if (wcrtomb(mbchar, arg, &mbstate) == WC_TO_MB_ERROR) { // See: man wcrtomb
        perror("print_wctomb(): Failed to convert a wchar_t!");
        return EOF;
    }
    return std::fputs(mbchar, stream);
}

int print_mbstring_to(const wchar_t *arg, std::FILE *stream)
{
    size_t len = std::wcslen(arg) + 1; // +1 for nul char.
    size_t bytes = MB_LEN_MAX * len; // Overkill but I am hella paranoid
    mbstate_t mbstate;
    int ret = EOF; // Return value of `std::fputs`, use `EOF` to signal errors.
    // operator `new` and `delete` won't link properly in C.
    char *mbstring = static_cast<char*>(std::malloc(bytes));
    if (mbstring == nullptr) {
        perror("Failed to allocate memory for mbstring");
        goto deinit;
    }
    std::memset(&mbstate, 0, sizeof(mbstate));
    len = std::wcsrtombs(mbstring, &arg, bytes, &mbstate); // Basically `strlen`
    if (len == WC_TO_MB_ERROR) {
        perror("wchar_t->char conversion failed");
        fputs("Is the locale correct? e.g. `setlocale(LC_CTYPE, \"\");\n", stderr);
        goto deinit;
    }
    ret = std::fputs(mbstring, stream);
deinit:
    std::free(mbstring);
    return ret;
}
