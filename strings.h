#ifndef STRINGS_H
#define STRINGS_H

#include "common.h"

typedef struct {
    const char *data;
    size_t      len;
} String;

typedef const char *CString;

#define string_from_literal(literal)    (String){literal, sizeof(literal) - 1}

#define STRING_FMTSPEC      "%.*s"
#define STRING_QFMTSPEC     "\"" STRING_FMTSPEC "\""

// For use with the `printf` format `"%.*s"`.
#define string_expand(string)   (int)(string).len, (string).data

// Assumes you'll never have a string this big!
#define STRING_NOT_FOUND    ((size_t)-1)

String
string_from_cstring(CString cstring);

String
string_slice(String string, size_t start, size_t stop);

size_t
string_index_substring(String haystack, String needle);

size_t
string_index_subcstring(String haystack, CString needle);

size_t
string_index_char(String haystack, char needle);

size_t
string_index_any_string(String haystack, String needle);

size_t
string_index_any_cstring(String haystack, CString needle);

#define string_index_any(haystack, needle) _Generic((needle),                  \
    String:       string_index_any_string,                                     \
    char *:       string_index_any_cstring,                                    \
    const char *: string_index_any_cstring                                     \
)(haystack, needle)

bool
string_split_iterator(String *state, char sep, String *current);

bool
string_split_lines_iterator(String *state, String *current);

#endif // STRINGS_H
