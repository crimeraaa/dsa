#ifndef STRINGS_H
#define STRINGS_H

#include "common.h"

typedef struct {
    const char *data;
    size_t      len;
} String;

// C99 compound literals have VERY different semantics in C++.
#ifdef __cplusplus
#define string_from_literal(literall)   {literal, sizeof(literal) - 1}
#else // !__cplusplus
#define string_from_literal(literal)    (String){literal, sizeof(literal) - 1}
#endif // __cplusplus

#define STRING_FMTSPEC      "%.*s"
#define STRING_QFMTSPEC     "\"" STRING_FMTSPEC "\""

// For use with the `printf` format `"%.*s"`.
#define string_expand(string)   (int)((string).len), ((string).data)

// Assumes you'll never have a string this big!
#define STRING_NOT_FOUND    ((size_t)-1)

bool
string_eq(String a, String b);

String
string_from_cstring(const char *cstring);

String
string_slice(String string, size_t start, size_t stop);

size_t
string_index_substring(String haystack, String needle);

size_t
string_index_subcstring(String haystack, const char *needle);

size_t
string_index_char(String haystack, char needle);

size_t
string_index_any_string(String haystack, String needle);

size_t
string_index_any_cstring(String haystack, const char *needle);

bool
string_split_char_iterator(String *state, String *current, char sep);

bool
string_split_string_iterator(String *state, String *current, String sep);

bool
string_split_cstring_iterator(String *state, String *current, const char *sep);

bool
string_split_lines_iterator(String *state, String *current);

#if defined(__STDC__) && __STDC_VERSION__ >= 201112L

#define string_index(haystack, needle)                                         \
_Generic((needle),                                                             \
    String:       string_index_substring,                                      \
    char *:       string_index_subcstring,                                     \
    const char *: string_index_subcstring,                                     \
)(haystack, needle)

#define string_split_iterator(state, current, sep)                             \
_Generic((sep),                                                                \
    String:       string_split_string_iterator,                                \
    char *:       string_split_cstring_iterator,                               \
    const char *: string_split_cstring_iterator,                               \
)(state, current, sep)

#define string_index_any(haystack, needle)                                     \
_Generic((needle),                                                             \
    String:       string_index_any_string,                                     \
    char *:       string_index_any_cstring,                                    \
    const char *: string_index_any_cstring                                     \
)(haystack, needle)

#endif // __STDC__VERSION__ == 201112L

#endif // STRINGS_H
