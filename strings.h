#ifndef STRINGS_H
#define STRINGS_H

#include "common.h"

typedef struct {
    const char *data;
    size_t      len;
} String;

#define string_from_literal(literal)    (String){literal, sizeof(literal) - 1}

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

#define string_index(haystack, needle) _Generic((needle),                      \
    String: string_index_substring,                                            \
    char *: string_index_subcstring,                                           \
    char:   string_index_char,                                                 \
    int:    string_index_char                                                  \
)(haystack, needle)

size_t
string_index_any_string(String haystack, String needle);

size_t
string_index_any_cstring(String haystack, const char *needle);

#define string_index_any(haystack, needle) _Generic((needle),                  \
    String: string_index_any_string,                                           \
    char *: string_index_any_cstring                                           \
)(haystack, needle)

bool
string_split_char_iterator(String *state, String *current, char sep);

bool
string_split_string_iterator(String *state, String *current, String sep);

bool
string_split_cstring_iterator(String *state, String *current, const char *sep);

#define string_split_iterator(state, current, sep) _Generic((sep),             \
    String: string_split_string_iterator,                                      \
    char *: string_split_cstring_iterator,                                     \
    char:   string_split_char_iterator,                                        \
    int:    string_split_char_iterator                                         \
)(state, current, sep)

bool
string_split_lines_iterator(String *state, String *current);

#endif // STRINGS_H
