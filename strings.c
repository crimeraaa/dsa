#include "strings.h"
#include "ascii.h"

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
