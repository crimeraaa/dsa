#pragma once

#ifdef DSA_IMPLEMENTATION
#define DSA_ASCII_IMPLEMENTATION
#endif // DSA_IMPLEMENTATION

#include "common.h"

bool
ascii_is_alpha(char ch);

bool
ascii_is_digit(char ch);

bool
ascii_is_alnum(char ch);

bool
ascii_is_upper(char ch);

bool
ascii_is_lower(char ch);

// https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Whitespace.html
bool
ascii_is_whitespace(char ch);

#ifdef DSA_ASCII_IMPLEMENTATION

bool
ascii_is_alpha(char ch)
{
    return ascii_is_upper(ch) || ascii_is_lower(ch);
}

bool
ascii_is_digit(char ch)
{
    return '0' <= ch && ch <= '9';
}

bool
ascii_is_alnum(char ch)
{
    return ascii_is_alpha(ch) || ascii_is_digit(ch);
}

bool
ascii_is_upper(char ch)
{
    return 'A' <= ch && ch <= 'Z';
}

bool
ascii_is_lower(char ch)
{
    return 'a' <= ch && ch <= 'z';
}

bool
ascii_is_whitespace(char ch)
{
    switch (ch) {
    case ' ':
    case '\r': // fallthrough
    case '\n': // fallthrough
    case '\t': // fallthrough
    case '\v': // fallthrough
    case '\f': return true;
    default:   return false;
    }
}

#endif // DSA_ASCII_IMPLEMENTATION
