#include <array>
#include <cassert>
#include <climits>
#include <cstdarg> /* va_list and accompaniying functions */
#include <cstdlib>
#include <cstdio> /* Too lazy to reimplement fputc, fputs and friends */
#include <type_traits>

#include "parse.h"

// Generally speaking, our largest integer types are 64-bits.
static constexpr size_t MAX_BINARY_LENGTH = (CHAR_BIT * sizeof(std::size_t));

static constexpr char g_digitchars[] = "0123456789abcdef";

/**
 * @brief   Reverse the contents of a nul-terminated C-string in place.
 */
char *reverse_string(char *buffer, size_t length)
{
    for (size_t i = 0, ii = length - 1; i < ii; i++, ii--) {
        char ch = buffer[i];
        buffer[i] = buffer[ii];
        buffer[ii] = ch;
    }
    return buffer;
}

template<typename IntT>
void write_integer(std::FILE *stream, IntT arg, int base = 10)
{
    static_assert(std::is_integral<IntT>::value, "bruh");
    if (arg == 0) {
        std::fputc('0', stream);
        return;
    }
    // Extras: 1 for nul char, 1 for dash, 2 for base.
    char buffer[MAX_BINARY_LENGTH + 4] = {0}; 
    size_t length = 0;
    bool negative = false;
    if constexpr(std::is_signed<IntT>::value) {
        if (arg < 0) {
            arg = std::abs(arg); // TODO: Fix conversion for INT_MIN and such
            negative = true;
        }
    }
    // Read the number from right to left, we'll reverse it later.
    // The result of module will give us the current rightmost digit,
    // which we can use to index into `g_digitchars`, conviniently enough.
    while (arg > 0) {
        buffer[length++] = g_digitchars[arg % base];
        arg /= base;
    }
    assert(length <= MAX_BINARY_LENGTH); // I hope this never happens
    if (negative) {
        buffer[length++] = '-';
    }
    // buffer was 0-initialized so very last slot should be '\0' itself.
    std::fputs(reverse_string(buffer, length), stream);
}

/**
 * @brief   Does the heavy lifting of parsing a single instance of a `%`.
 *          Lots of work goes into this, see C's printf spec.
 * @param   stream      Where the output is heading.
 * @param   ptr         Current position in the format string.
 * @param   args        Variadic argument list on the stack. Pop as we go!
 * @param   spec        The current format specifier we have.
 */
void parse_fmt(std::FILE *stream, const char *ptr, std::va_list args, char spec)
{
    // TODO: Parse modifiers to determine if need, say, long int or wchar string
    (void)ptr;
    // Print lone format specifiers (no modifiers)
    switch (spec)
    {
        case '%': {
            break;
        }
        case 'c': {
            // va_arg promotes char to int so get an int then cast down
            std::fputc(va_arg(args, int), stream);
            break;
        }
        case 'd': // fall-through
        case 'i': {
            write_integer(stream, va_arg(args, int));
            break;
        }
        case 'u': {
            write_integer(stream, va_arg(args, unsigned));
            break;
        }
        case 'f': {
            // Floats are promoted to double automatically.
            double f = va_arg(args, double);
            (void)f; // ignore for now, idk what to do
            break;
        }
        case 's': {
            std::fputs(va_arg(args, char*), stream);
            break;
        }
        case 'p': {
            // TODO: Figure out how to print a memory address
            void *p = va_arg(args, void*);
            (void)p;
            break;
        }
        default: {
            break;
        }
    }
}

void write_format(std::FILE *stream, const char *fmts, ...)
{
    const char *ptr = fmts; // Keep track of current char
    std::va_list args;
    bool is_fmtspec = false;
    char ch;

    va_start(args, fmts); // args now points to first vararg, whatever it may be
    // Parse until NUL char is reached
    // ptr itself is incremented but postfix returns the original value
    while ((ch = *ptr++) != '\0') {
        if (ch == '%') {
            is_fmtspec = true;
            ch = *ptr++; // was already incremented so go to next token
        }
        if (!is_fmtspec) {
            std::fputc(ch, stream);
        } else {
            parse_fmt(stream, ptr, args, ch);
            is_fmtspec = false;
        }
    }
    va_end(args);
}
