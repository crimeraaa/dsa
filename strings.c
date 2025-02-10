#include "strings.h"

#include <assert.h> // assert
#include <string.h> // strlen

String
string_from_cstring(CString cstring)
{
    String string = {cstring, cast(int)strlen(cstring)};
    return string;
}

String
string_slice(String string, int start, int stop)
{
    /* 
       String msg = {data = "Hi mom!", len = 7}
       string_slice(string = msg, start = -1, stop = -1)

       since (start = -1) < 0
            start = (string.len = 7) + (start = -1)
                  = 6
        
        since (stop = -1) < 0
            stop = (string.len = 7) + (stop = -1) + 1
                 = 6
     */
    if (start < 0) start += string.len;
    if (stop < 0) stop += string.len;
    assert(0 <= start && start < string.len);
    assert(0 <= stop  && stop  <= string.len);
    String slice = {&string.data[start], stop - start};
    return slice;
}

int
string_index_substring(String haystack, String needle)
{
    if (needle.len == 1) return string_index_char(haystack, needle.data[0]);

    for (int offset = 0; offset < haystack.len; offset++) {
        // Substring cannot possibly be found in the remaining string?
        if (offset + needle.len >= haystack.len) return -1;

        bool found = true;
        for (int j = 0; j < needle.len; j++) {
            if (haystack.data[offset + j] != needle.data[j]) {
                found = false;
                break;
            }
            // Implicitly keep going while matches, leaving `found` as-is.
        }
        if (found) return offset;
    }
    return -1;
}

int
string_index_subcstring(String haystack, CString needle)
{
    return string_index_substring(haystack, string_from_cstring(needle));
}

int
string_index_char(String haystack, char needle)
{
    for (int i = 0; i < haystack.len; i++) {
        if (haystack.data[i] == needle) return i;
    }
    return -1;
}

int
string_index_any_string(String haystack, String needle)
{
    for (int j = 0; j < needle.len; j++) {
        int i = string_index_char(haystack, needle.data[j]);
        if (i != -1) return i;
    }
    return -1;
}

int
string_index_any_cstring(String haystack, CString needle)
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
    if (state->len == 0) return false;

    int index = string_index_char(*state, sep);

    // If `index == -1`, we match the remainder of the string.
    *current  = string_slice(*state, 0, index == -1 ? state->len : index);
    int start, stop;
    // Can we can safely slice with start = index + 1?
    if (index != -1 && index + 1 < state->len) {
        start = index + 1;
        stop  = state->len;
    }
    // Otherwise, create a zero-length slice. This indicates that the next
    // iteration will cause the loop to terminate.
    else {
        start = stop = -1;
    }
    *state = string_slice(*state, start, stop);
    return true;
}
