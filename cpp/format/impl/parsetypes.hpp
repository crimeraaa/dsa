#pragma once

#include <cstddef> /* std::size_t */
#include <climits> /* CHAR_BIT macro */

// Generally speaking, our largest integer types are 64-bits.
constexpr size_t MAX_BINARY_LENGTH = (CHAR_BIT * sizeof(std::size_t));

// `constexpr` implies `const` which implies `static` (internal linkage).
constexpr char g_digitchars[] = "0123456789abcdef";


union FmtSigned {
    int i; // `char` and `short` are also promoted to this
    long li;
    long long lli;
};

union FmtUnsigned {
    unsigned int u; // `unsigned char` and `unsigned short` get promoted to this
    unsigned long lu;
    unsigned long long llu;
};

enum class FmtMod {
    is_none,
    is_long, // `"%l<spec>"`: long, unsigned long, wchar_t*
    is_long_long, // `"%ll<spec>"`: long long, unsigned long long
    is_short, // `"%h<spec>"`: short, unsigned short
    is_short_short, // `"%hh<spec>"`: unsigned char, mainly used with `"%hhx"`.
};

union FmtValue {
    FmtSigned si;
    FmtUnsigned ui;
    double df; // `float` is also promoted to this
    char *cs; 
    wchar_t *wcs;
    void *ptr; // Raw memory address
};

struct FmtSpec {
    const char *nextptr; // Pointer to next non-specifier/modifier character.
    FmtMod mod;
    char tag; // Primary format specifier like `'i'`  in `"%-08lli"`.
};
