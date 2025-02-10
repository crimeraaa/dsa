#ifndef STRINGS_H
#define STRINGS_H

#include <stdbool.h>

#include "common.h"

typedef struct {
    const char *data;
    int len;
} String;

typedef const char *CString;

#define string_from_literal(literal)    (String){literal, cast(int)sizeof(literal) - 1}

#define STRING_FMTSPEC  "%.*s"
#define STRING_QFMTSPEC "\"" STRING_FMTSPEC "\""

// For use with the `printf` format `"%.*s"`.
#define string_expand(string)   (string).len, (string).data

String
string_from_cstring(CString cstring);

String
string_slice(String string, int start, int stop);

int
string_index_substring(String haystack, String needle);

int
string_index_subcstring(String haystack, CString needle);

int
string_index_char(String haystack, char needle);

int
string_index_any_string(String haystack, String needle);

int
string_index_any_cstring(String haystack, CString needle);

#define string_index_any(haystack, needle) _Generic((needle),                  \
    String: string_index_any_string,                                           \
    char:   string_index_char,                                                 \
    char *: string_index_any_cstring                                           \
)(haystack, needle)

bool
string_split_iterator(String *state, char sep, String *current);

bool
string_split_lines_iterator(String *state, String *current);

#endif // STRINGS_H
