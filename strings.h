#ifndef STRINGS_H
#define STRINGS_H

#include "common.h"
#include "allocator.h"

typedef struct {
    const char *data;
    size_t      len;
} String;

/**
 * @details
 *      The `_first_` nonsense is to allow us to create a for loop that runs
 *      exactly only once.
 *
 *      This is useful because we can't declare variables of multiple unique
 *      types within a single statement in C (e.g. `int x = 0, float f = 1.1`).
 *
 *      `char ch, *ptr = str.data, *end = ptr + str.len` is not fine because it
 *      casts away constness!
 *
 *      `const char ch, *ptr, *end` doesn't work because now `ch` is not mutable.
 */
#define string_for_each(name, string)                                          \
for (const char *_ptr_ = (string).data, *const _end_ = _ptr_ + (string).len;   \
    _ptr_ < _end_;                                                             \
    ++_ptr_)                                                                   \
    for (char name = *_ptr_, _first_ = 1; _first_; _first_ = 0)

#define string_for_eachi(idx, string) \
for (size_t idx = 0, _end_ = (string).len; idx < _end_; ++idx)

// C99 compound literals have VERY different semantics in C++.
#ifdef __cplusplus
#define string_literal(literal)     {literal, sizeof(literal) - 1}
#else // !__cplusplus
#define string_literal(literal)     (String){literal, sizeof(literal) - 1}
#endif // __cplusplus

#define STRING_FMTSPEC          "%.*s"
#define STRING_QFMTSPEC         "\'" STRING_FMTSPEC "\'"
#define STRING_FMTARG(string)   ((int)((string).len)), ((string).data)

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

/**
 * @param comparison
 *      Can be `false` or `0` so that you get the first index of the first
 *      character that does NOT satisfy `callback`. You can also think of this
 *      as 'negating' character classes.
 */
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
string_split_iterator_fn(String *current, String *state, bool (*callback)(char ch));

bool
string_split_char_iterator(String *current, String *state, char sep);

bool
string_split_string_iterator(String *current, String *state, String sep);

bool
string_split_cstring_iterator(String *current, String *state, const char *sep);

bool
string_split_lines_iterator(String *current, String *state);

bool
string_split_whitespace_iterator(String *current, String *state);

// }}} -------------------------------------------------------------------------

#if defined(__STDC__) && __STDC_VERSION__ >= 201112L

#define string_index(haystack, needle)                                         \
_Generic((needle),                                                             \
    String:     string_index_substring,                                        \
    default:    string_index_subcstring                                        \
)(haystack, needle)

#define string_split_iterator(current, state, sep)                             \
_Generic((sep),                                                                \
    String:     string_split_string_iterator,                                  \
    default:    string_split_cstring_iterator                                  \
)(current, state, sep)

#define string_index_any(haystack, charset)                                    \
_Generic((charset),                                                            \
    String:     string_index_any_string,                                       \
    default:    string_index_any_cstring,                                      \
)(haystack, charset)

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

size_t
string_builder_len(String_Builder *builder);

void
string_builder_reset(String_Builder *builder);

Allocator_Error
string_append_char(String_Builder *builder, char ch);

Allocator_Error
string_append_string(String_Builder *builder, String text);

Allocator_Error
string_append_cstring(String_Builder *builder, const char *text);

Allocator_Error
string_prepend_char(String_Builder *builder, char ch);

Allocator_Error
string_prepend_string(String_Builder *builder, String text);

Allocator_Error
string_prepend_cstring(String_Builder *builder, const char *text);

String
string_to_string(const String_Builder *builder);

const char *
string_to_cstring(const String_Builder *builder);

// }}} -------------------------------------------------------------------------

#endif // STRINGS_H
