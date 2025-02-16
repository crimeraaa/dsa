#include "strings.h"

#include <assert.h> // assert
#include <string.h> // strlen

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

size_t
string_index_substring(String haystack, String needle)
{
    if (needle.len == 1)
        return string_index_char(haystack, needle.data[0]);

    for (size_t offset = 0; offset < haystack.len; offset++) {
        // Substring cannot possibly be found in the remaining string?
        if (offset + needle.len >= haystack.len) break;

        bool found = true;
        for (size_t j = 0; j < needle.len; j++) {
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
    for (size_t i = 0; i < haystack.len; i++) {
        if (haystack.data[i] == needle)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_any_string(String haystack, String needle)
{
    for (size_t j = 0; j < needle.len; j++) {
        size_t i = string_index_char(haystack, needle.data[j]);
        if (i != STRING_NOT_FOUND)
            return i;
    }
    return STRING_NOT_FOUND;
}

size_t
string_index_any_cstring(String haystack, const char *needle)
{
    return string_index_any_string(haystack, string_from_cstring(needle));
}

bool
string_split_lines_iterator(String *state, String *current)
{
    return string_split_iterator(state, '\n', current);
}

bool
string_split_iterator(String *state, char sep, String *current)
{
    if (state->len == 0)
        return false;

    size_t index = string_index_char(*state, sep);

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
