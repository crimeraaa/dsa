#ifndef STRINGS_H
#define STRINGS_H

#include "common.h"
#include "allocator.h"

typedef struct {
    const char *data;
    size_t      len;
} String;

#define string_for_each(ptr, string) \
for (const char *ptr = (string).data, *const _end_ = ptr + (string).len; \
    ptr < _end_; \
    ++ptr)

#define string_for_eachi(idx, string) \
for (size_t idx = 0, _end_ = (string).len; idx < _end_; ++idx)

// C99 compound literals have VERY different semantics in C++.
#ifdef __cplusplus
#define string_literal(literall)    {literal, sizeof(literal) - 1}
#else // !__cplusplus
#define string_literal(literal)     (String){literal, sizeof(literal) - 1}
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

// Returns a slice of `text` that does not include any whitespace to either side.
String
string_trim_space(String text);

String
string_trim_left_space(String text);

String
string_trim_right_space(String text);

String
string_trim_left_fn(String text, bool (*callback)(char ch));

String
string_trim_right_fn(String text, bool (*callback)(char ch));

// LEFT INDEX FUNCTIONS ---------------------------------------------------- {{{

size_t
string_index_fn(String text, bool (*callback)(char ch), bool comparison);

size_t
string_index_substring(String haystack, String needle);

size_t
string_index_subcstring(String haystack, const char *needle);

size_t
string_index_char(String haystack, char needle);

size_t
string_index_any_string(String haystack, String charset);

size_t
string_index_any_cstring(String haystack, const char *charset);

// }}} -------------------------------------------------------------------------

// RIGHT INDEX FUNCTIONS --------------------------------------------------- {{{

size_t
string_last_index_fn(String text, bool (*callback)(char ch), bool comparison);

size_t
string_last_index_char(String haystack, char needle);

size_t
string_last_index_any_string(String haystack, String charset);

size_t
string_last_index_any_cstring(String haystack, const char *charset);

// }}} -------------------------------------------------------------------------

// SPLIT ITERATORS --------------------------------------------------------- {{{

bool
string_split_char_iterator(String *state, String *current, char sep);

bool
string_split_any_string_iterator(String *state, String *current, String charset);

bool
string_split_string_iterator(String *state, String *current, String sep);

bool
string_split_cstring_iterator(String *state, String *current, const char *sep);

bool
string_split_lines_iterator(String *state, String *current);

// }}} -------------------------------------------------------------------------

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

// STRING BUILDER ---------------------------------------------------------- {{{

typedef struct {
    Allocator allocator;
    char     *buffer;
    size_t    len;
    size_t    cap;
} String_Builder;

String_Builder
string_builder_make(Allocator allocator);

String_Builder
string_builder_make_fixed(char *buffer, size_t cap);

void
string_builder_reset(String_Builder *builder);

bool
string_builder_append_char(String_Builder *builder, char ch);

bool
string_builder_append_string(String_Builder *builder, String text);

bool
string_builder_append_cstring(String_Builder *builder, const char *text);

String
string_builder_to_string(String_Builder *builder);

const char *
string_builder_to_cstring(String_Builder *builder);

// }}} -------------------------------------------------------------------------

#endif // STRINGS_H
