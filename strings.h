#pragma once

#include "common.h"
#include "mem/allocator.h"

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
#define string_fmtarg(string)   ((int)((string).len)), ((string).data)

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

#ifdef STRINGS_IMPLEMENTATION

#include "ascii.h"  // ascii_is_whitespace

#include <assert.h> // assert
#include <string.h> // strlen, memcmp

bool
string_eq(String a, String b)
{
    // Fast path #1: Strings of differing lengths are never equivalent.
    if (a.len != b.len)
        return false;

    // Fast path #2: Empty strings (regardless of data) are always equivalent.
    if (a.len == 0)
        return true;

    // Fast path #3: Strings of the same length and data are always equivalent.
    if (a.data == b.data)
        return true;

    // Likely optimized to read machine words (e.g. 8-bytes) at a time.
    return memcmp(a.data, b.data, a.len) == 0;
}

String
string_from_cstring(const char *cstring)
{
    String string = {cstring, strlen(cstring)};
    return string;
}

String
string_slice(String string, size_t start, size_t stop)
{
    // You can *point* at string.data[string.len], but you cannot deref.
    assert(0 <= start && start <= string.len);
    assert(0 <= stop  && stop  <= string.len);
    assert(start <= stop);

    String slice = {string.data + start, stop - start};
    return slice;
}

String
string_trim_space(String text)
{
    return string_trim_right_space(string_trim_left_space(text));
}

String
string_trim_left_space(String text)
{
    return string_trim_left_fn(text, &ascii_is_whitespace);
}

String
string_trim_right_space(String text)
{
    return string_trim_right_fn(text, &ascii_is_whitespace);
}

String
string_trim_left_fn(String text, bool (*callback)(char ch))
{
    size_t index = string_index_fn(text, callback, false);
    if (index == STRING_NOT_FOUND)
        return string_slice(text, 0, 0);
    return string_slice(text, index, text.len);
}

String
string_trim_right_fn(String text, bool (*callback)(char ch))
{
    // If `index` is `STRING_NOT_FOUND`, adding 1 will overflow to 0.
    // We want this so we can return an empty string in such a case.
    size_t index = string_last_index_fn(text, callback, false) + 1;
    return string_slice(text, 0, index);
}

// LEFT INDEX FUNCTIONS ---------------------------------------------------- {{{

size_t
string_index_fn(String text, bool (*callback)(char ch), bool comparison)
{
    string_for_eachi(index, text) {
        if (callback(text.data[index]) == comparison)
            return index;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_substring(String haystack, String needle)
{
    if (needle.len == 1)
        return string_index_char(haystack, needle.data[0]);

    string_for_eachi(offset, haystack) {
        // Substring cannot possibly be found in the remaining string?
        if (offset + needle.len >= haystack.len) break;

        bool found = true;
        string_for_eachi(j, needle) {
            if (haystack.data[offset + j] != needle.data[j]) {
                found = false;
                break;
            }
            // Implicitly keep going while matches, leaving `found` as-is.
        }
        if (found)
            return offset;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_subcstring(String haystack, const char *needle)
{
    return string_index_substring(haystack, string_from_cstring(needle));
}

size_t
string_index_char(String haystack, char needle)
{
    string_for_eachi(i, haystack) {
        if (haystack.data[i] == needle)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_any_string(String haystack, String charset)
{
    string_for_each(expected, charset) {
        size_t i = string_index_char(haystack, expected);
        if (i != STRING_NOT_FOUND)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_any_cstring(String haystack, const char *charset)
{
    return string_index_any_string(haystack, string_from_cstring(charset));
}

// }}} -------------------------------------------------------------------------

// RIGHT INDEX FUNCTIONS --------------------------------------------------- {{{

size_t
string_last_index_fn(String text, bool (*callback)(char ch), bool comparison)
{
    // Once we overflow, bail out.
    for (size_t i = text.len - 1; i < STRING_NOT_FOUND; --i) {
        if (callback(text.data[i]) == comparison)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_last_index_char(String haystack, char needle)
{
    // Once we overflow, bail out.
    for (size_t i = haystack.len - 1; i < STRING_NOT_FOUND; --i) {
        if (haystack.data[i] == needle)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_last_index_any_string(String haystack, String charset)
{
    string_for_each(expected, charset) {
        size_t i = string_last_index_char(haystack, expected);
        if (i != STRING_NOT_FOUND)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_last_index_any_cstring(String haystack, const char *charset)
{
    return string_last_index_any_string(haystack, string_from_cstring(charset));
}

// }}} -------------------------------------------------------------------------

// SPLIT ITERATORS --------------------------------------------------------- {{{

bool
string_split_lines_iterator(String *current, String *state)
{
    return string_split_char_iterator(state, current, '\n');
}

bool
string_split_whitespace_iterator(String *current, String *state)
{
    size_t start = STRING_NOT_FOUND;
    size_t stop  = STRING_NOT_FOUND;
    String view  = *state;
    string_for_eachi(index, view) {
        stop = index;
        char ch = view.data[index];
        if (ascii_is_whitespace(ch)) {
            // I would love a signed size type right about now...
            if (start != STRING_NOT_FOUND) {
                *current = string_slice(view, start, stop);
                *state   = string_slice(view, stop, view.len);
                return true;
            }
        } else {
            /**
             * Only save the starting index now so that we can toss out ALL
             * non-whitespace before the first valid character. This is useful
             * if the user typed multiple whitespace between words.
             *
             * @example
             *      "hi   mom"
             *      [1]: "hi"
             *      [2]: "   mom" => "mom"
             */
            if (start == STRING_NOT_FOUND)
                start = stop;
        }
    }

    // We didn't even iterate? This means `state` is already exhausted.
    if (stop == STRING_NOT_FOUND || start == STRING_NOT_FOUND) {
        *current = string_slice(view, view.len, view.len);
        return false;
    }

    // We probably ended the loop early.
    *current = string_slice(view, start, view.len);
    *state   = string_slice(view, view.len, view.len);
    return true;
}

bool
string_split_cstring_iterator(String *current, String *state, const char *sep)
{
    return string_split_string_iterator(state, current, string_from_cstring(sep));
}

static bool
_string_split_iterator(String *current, String *state, size_t index)
{
    // If `index == STRING_NOT_FOUND`, we match the remainder of the string.
    *current = string_slice(*state, 0, (index == STRING_NOT_FOUND) ? state->len : index);
    size_t start, stop;
    // Can we can safely slice with start = index + 1?
    if (index != STRING_NOT_FOUND && index + 1 < state->len) {
        start = index + 1;
        stop  = state->len;
    }
    // Otherwise, create a zero-length slice that points to the very end.
    // This indicates that the next iteration will cause the loop to terminate.
    else {
        start = state->len;
        stop  = start;
    }
    *state = string_slice(*state, start, stop);
    return true;
}

bool
string_split_iterator_fn(String *current, String *state, bool (*callback)(char ch))
{
    if (state->len == 0)
        return false;

    size_t index = string_index_fn(*state, callback, true);
    return _string_split_iterator(state, current, index);
}

bool
string_split_char_iterator(String *current, String *state, char sep)
{
    if (state->len == 0)
        return false;

    size_t index = string_index_char(*state, sep);
    return _string_split_iterator(state, current, index);
}

bool
string_split_string_iterator(String *current, String *state, String sep)
{
    if (state->len == 0)
        return false;

    size_t index = string_index_substring(*state, sep);
    return _string_split_iterator(state, current, index);
}

// }}} -------------------------------------------------------------------------

#endif // STRING_IMPLEMENTATION

#ifdef STRINGS_BUILDER_IMPLEMENTATION

#include "strings_builder.h"

#endif // STRINGS_BUILDER_IMPLEMENTATION
