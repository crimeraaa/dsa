#include "strings.h"

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

// https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Whitespace.html
static bool
is_whitespace(char ch)
{
    switch (ch) {
    case ' ':
    case '\t': // horizontal tab
    case '\f': // form-feed
    case '\v': // vertical-tab
    case '\r': // carriage-return
    case '\n': // linefeed
        return true;
    default:
        return false;
    }
}

String
string_trim_left_space(String text)
{
    return string_trim_left_fn(text, &is_whitespace);
}

String
string_trim_right_space(String text)
{
    return string_trim_right_fn(text, &is_whitespace);
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
    string_for_each(ptr, charset) {
        size_t i = string_index_char(haystack, *ptr);
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
    string_for_each(ptr, charset) {
        size_t i = string_last_index_char(haystack, *ptr);
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
string_split_lines_iterator(String *state, String *current)
{
    return string_split_char_iterator(state, current, '\n');
}

bool
string_split_cstring_iterator(String *state, String *current, const char *sep)
{
    return string_split_string_iterator(state, current, string_from_cstring(sep));
}

static bool
_string_split_iterator(String *state, String *current, size_t index)
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
string_split_char_iterator(String *state, String *current, char sep)
{
    if (state->len == 0)
        return false;

    size_t index = string_index_char(*state, sep);
    return _string_split_iterator(state, current, index);
}

bool
string_split_string_iterator(String *state, String *current, String sep)
{
    if (state->len == 0)
        return false;

    size_t index = string_index_substring(*state, sep);
    return _string_split_iterator(state, current, index);
}

// }}} -------------------------------------------------------------------------
