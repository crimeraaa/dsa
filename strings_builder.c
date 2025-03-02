#include <string.h>

#include "strings.h"

String_Builder
string_builder_make(Allocator allocator)
{
    String_Builder builder = {
        .allocator = allocator,
        .buffer    = NULL,
        .len       = 0,
        .cap       = 0,
    };
    return builder;
}

String_Builder
string_builder_make_fixed(char *buffer, size_t cap)
{
    String_Builder builder = {
        .allocator = NIL_ALLOCATOR,
        .buffer    = buffer,
        .len       = 0,
        .cap       = cap,
    };
    return builder;
}

void
string_builder_reset(String_Builder *builder)
{
    builder->len = 0;
}

bool
string_append_char(String_Builder *builder, char ch)
{
    char   tmp[] = {ch};
    String hack  = {tmp, sizeof tmp};
    return string_append_string(builder, hack);
}

bool
string_append_string(String_Builder *builder, String text)
{
    size_t len    = builder->len;
    size_t end    = len + text.len;
    size_t cap    = builder->cap;
    char  *buffer = builder->buffer;
    // Need to fit resulting text along with nul termination as well.
    if (end + 1 >= cap) {
        size_t new_cap = (cap == 0) ? 8 : cap * 2;
        char  *new_buffer = mem_resize(char, buffer, cap, new_cap, builder->allocator);
        if (new_buffer == NULL)
            return false;
        buffer          = cast(char *)memcpy(new_buffer, buffer, cap);
        builder->buffer = buffer;
        builder->cap    = new_cap;
    }
    builder->len = end;
    buffer[end]  = '\0';
    memcpy(&buffer[len], text.data, text.len);
    return true;
}

bool
string_append_cstring(String_Builder *builder, const char *text)
{
    return string_append_string(builder, string_from_cstring(text));
}


String
string_to_string(const String_Builder *builder)
{
    String result = {builder->buffer, builder->len};
    return result;
}

const char *
string_to_cstring(const String_Builder *builder)
{
    return builder->buffer;
}
