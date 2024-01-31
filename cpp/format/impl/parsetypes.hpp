#pragma once

enum class FmtLen {
    is_none,
    is_long,        // `"%l<spec>"` : long, unsigned long, wchar_t, wchar_t*
    is_long_long,   // `"%ll<spec>"`: long long, unsigned long long
    is_short,       // `"%h<spec>"` : short, unsigned short
    is_short_short, // `"%hh<spec>"`: unsigned char, mainly used with `"%hhx"`.
};

/**
 * Not sure what I'd use this for at the moment. This is meant to represent
 * most possible variables under the C printf format specifiers.
 */
union FmtValue {
    int i; // automatic promotion of char/wchar_t (signedness not guaranteed)
    long li;
    long long lli;
    unsigned int u; // automatic promotion of unsigned char/wchar_t.
    unsigned long lu;
    unsigned long long llu;
    char *s;
    wchar_t *ls;
    double f; // `%lf` is redundant as `float` is always promoted to `double`.
    long double Lf; // `%llf` is a GNU extension.
    void *p;
};

struct FmtParse {
    const char *endptr; // Pointer to the last specifier/modifier for this.
    FmtLen len;
    char spec; // Primary format specifier like `'i'`  in `"%-08lli"`.
    int base; // For integer types only
    bool is_signed; // For integer types only
};
