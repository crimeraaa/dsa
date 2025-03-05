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
        .allocator = GLOBAL_NONE_ALLOCATOR,
        .buffer    = buffer,
        .len       = 0,
        .cap       = cap,
    };
    return builder;
}

size_t
string_builder_len(String_Builder *builder)
{
    return builder->len;
}

void
string_builder_reset(String_Builder *builder)
{
    builder->len = 0;
}

Allocator_Error
string_append_char(String_Builder *builder, char ch)
{
    char   tmp[] = {ch};
    String hack  = {tmp, sizeof tmp};
    return string_append_string(builder, hack);
}

static Allocator_Error
_string_builder_check_resize(String_Builder *builder, size_t extra)
{
    size_t cap = builder->cap;
    // Need to fit resulting text along with nul termination as well.
    if (builder->len + extra + 1 >= cap) {
        size_t new_cap = (cap == 0) ? 8 : cap * 2;
        char  *new_buffer = mem_resize(char, builder->buffer, cap, new_cap, builder->allocator);
        if (new_buffer == NULL)
            return ALLOCATOR_ERROR_OUT_OF_MEMORY;
        builder->buffer = cast(char *)memcpy(new_buffer, builder->buffer, cap);
        builder->cap    = new_cap;
    }
    return ALLOCATOR_ERROR_NONE;
}

Allocator_Error
string_append_string(String_Builder *builder, String text)
{
    Allocator_Error err = _string_builder_check_resize(builder, text.len);
    if (err != ALLOCATOR_ERROR_NONE)
        return err;
    memcpy(&builder->buffer[builder->len], text.data, text.len);
    builder->buffer[builder->len += text.len] = '\0';
    return ALLOCATOR_ERROR_NONE;
}

Allocator_Error
string_append_cstring(String_Builder *builder, const char *text)
{
    return string_append_string(builder, string_from_cstring(text));
}

Allocator_Error
string_prepend_char(String_Builder *builder, char ch)
{
    String tmp = {&ch, 1};
    return string_prepend_string(builder, tmp);
}

Allocator_Error
string_prepend_string(String_Builder *builder, String text)
{
    Allocator_Error err = _string_builder_check_resize(builder, text.len);
    if (err != ALLOCATOR_ERROR_NONE)
        return err;

    // Move the old text to the new location.
    // e.g. given "hi mom!" (len = 7), prepend "yay " (len = 4)
    //      1. Resize builder to be "hi mom!1234" (len = 11) NOTE: have garbage!
    //      2. Move old text "hi mom!" to new location (4): "hi mhi mom!"
    //      3. Copy new text to old location (0): "yay hi mom!"
    memmove(&builder->buffer[text.len], &builder->buffer[0], builder->len);
    memcpy(&builder->buffer[0],        text.data, text.len);
    builder->buffer[builder->len += text.len] = '\0';
    return ALLOCATOR_ERROR_NONE;
}

Allocator_Error
string_prepend_cstring(String_Builder *builder, const char *text)
{
    return string_prepend_string(builder, string_from_cstring(text));
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
