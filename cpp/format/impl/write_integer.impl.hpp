#pragma once

#include <cassert> /* assert macro */
#include <cstdio> /* std::fputc, std::fputs, std::FILE * */
#include <cstdlib> /* std::abs overloads, std::size_t */
#include <type_traits> /* std::is_integral, std::is_signed */

#include "parsetypes.hpp"

/**
 * @brief   Helper function for the internal loop of `write_integer`.
 *          This is because we want negative numbers to also reach towards 0.
 *          But since they're negative, they're less than 0.
 */
template<typename IntT>
bool write_integer_loop(IntT arg)
{
    if constexpr(std::is_signed<IntT>::value) {
        if (arg < 0) {
            return arg < 0;
        }
    } 
    return arg > 0;
}

/**
 * @brief   (Try to) Write the correct base prefix to `buffer`.
 * 
 * @param   buffer      Stack-allocated array of chars to write to.
 * @param   length      How many non-nul `char`s were written so far.
 * @param   negative    Used to determine if we should append a `'-'`.
 * @param   base        (Will be) used to append a base prefix like `"0x"`.
 * 
 * @warning We're working with raw C-pointers, `length` may overrun `buffer`!
 * 
 * @return  New length of the buffer, else `0` if `base` was unknown.
 *
 * @note    The buffer was written from right to left so we do the same.
 *
 * @bug     So far, this function probably won't work probably for negatives
 *          being represented in binary and hexadecimal formats.
 */
template<typename CharT>
size_t prefix_integer(CharT *buffer, size_t length, bool negative, int base)
{
    CharT prefix;
    switch (base)
    {
        case 2: {
            prefix = 'b';
            break;
        }
        case 8: {
            prefix = 'o';
            break;
        }
        case 10: {
            break;
        }
        case 16: {
            prefix = 'x';
            break;
        }
        default: {
            assert(false && "prefix_integer(): Unknown base!");
            return 0;
        }
    }
    // Remember that we're writing from right to left, so use "x0" or "b0".
    // It will be reversed later on to show the correct string.
    if (base != 10) {
        buffer[length++] = prefix;
        buffer[length++] = '0';
    }
    if (negative) {
        buffer[length++] = '-';
    }
    return length;
}

/**
 * @brief   Reverse the contents of a nul-terminated C-string in place.
 *          This is used by functions that write numbers from right to left.
 */
template<typename CharT>
CharT *reverse_string(CharT *buffer, size_t length)
{
    for (size_t i = 0, ii = length - 1; i < ii; i++, ii--) {
        CharT ch = buffer[i];
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
            negative = true;
        }
    }
    // Read the number from right to left, we'll reverse it later.
    while (write_integer_loop(arg)) {
        // Rightmost digit is also index into g_digitchars, but keep it positive
        int indexchar = std::abs(static_cast<int>(arg % base)); 
        buffer[length++] = g_digitchars[indexchar];
        arg /= base;
    }
    assert(length <= MAX_BINARY_LENGTH && "write_integer(): Too many digits!"); 
    if (negative) {
        buffer[length++] = '-';
    }
    // buffer was 0-initialized so very last slot should be '\0' itself.
    std::fputs(reverse_string(buffer, length), stream);
}
