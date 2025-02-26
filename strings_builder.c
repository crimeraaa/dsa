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
string_builder_append_char(String_Builder *builder, char ch)
{
    char   tmp[] = {ch};
    String hack  = {tmp, sizeof tmp};
    return string_builder_append_string(builder, hack);
}

bool
string_builder_append_string(String_Builder *builder, String text)
{
    size_t end = builder->len + text.len;
    // Need to fit resulting text along with nul termination as well.
    if (end + 1 >= builder->cap) {
        size_t new_cap = (builder->cap == 0) ? 8 : builder->cap * 2;
        char  *new_buffer = mem_resize(char, builder->buffer, builder->cap, new_cap, builder->allocator);
        if (new_buffer == NULL)
            return false;
        builder->buffer = cast(char *)memcpy(new_buffer, builder->buffer, builder->cap);
        builder->cap    = new_cap;
    }
    memcpy(&builder->buffer[builder->len], text.data, text.len);
    builder->buffer[end] = '\0';
    builder->len = end;
    return true;
}

bool
string_builder_append_cstring(String_Builder *builder, const char *text)
{
    return string_builder_append_string(builder, string_from_cstring(text));
}


String
string_builder_to_string(String_Builder *builder)
{
    String result = {builder->buffer, builder->len};
    return result;
}

const char *
string_builder_to_cstring(String_Builder *builder)
{
    return builder->buffer;
}
