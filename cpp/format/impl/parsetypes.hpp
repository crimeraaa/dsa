#pragma once

#include <cstddef> /* std::size_t */
#include <climits> /* CHAR_BIT macro */

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

enum class FmtLen {
    is_none,
    is_long, // `"%l<spec>"`: long, unsigned long, wchar_t, wchar_t*
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

struct FmtParse {
    const char *endptr; // Pointer to the last specifier/modifier for this.
    FmtLen len;
    char spec; // Primary format specifier like `'i'`  in `"%-08lli"`.
    int base; // For integer types only
    bool is_signed; // For integer types only
};
