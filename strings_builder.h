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
    // The memset is very important to ensure nul-termination.
    String_Builder builder = {
        .allocator = GLOBAL_NONE_ALLOCATOR,
        .buffer    = cast(char *)memset(buffer, 0, cap),
        .len       = 0,
        .cap       = cap,
    };
    return builder;
}

void
string_builder_destroy(String_Builder *builder)
{
    mem_delete(builder->buffer, builder->cap, builder->allocator);
}

size_t
string_builder_len(String_Builder *builder)
{
    return builder->len;
}

void
string_builder_reset(String_Builder *builder)
{
    // This is necessary to ensure future writes are nul-terminated.
    memset(&builder->buffer[0], 0, builder->cap);
    builder->len = 0;
}

Allocator_Error
string_append_char(String_Builder *builder, char ch)
{
    String tmp = {&ch, 1};
    return string_append_string(builder, tmp);
}

static Allocator_Error
_string_builder_check_resize(String_Builder *builder, size_t extra)
{
    size_t cap = builder->cap;
    size_t len = builder->len;
    // Need to fit resulting text along with nul termination as well.
    if (len + extra + 1 >= cap) {
        size_t          new_cap = (cap == 0) ? 8 : cap * 2;
        Allocator_Error error;
        char           *new_buffer = mem_resize(char, &error, builder->buffer, cap, new_cap, builder->allocator);
        if (error)
            return error;

        // We assume that allocators that fulfill resize requests already copy
        // over the old data to the new buffer.
        builder->buffer = new_buffer;

        // Zero out the newly acquired region. Also helps with nul-termination.
        // NOTE: memset with length 0 is defined as long as the pointer is valid.
        memset(&new_buffer[len], 0, new_cap - len);
        builder->cap = new_cap;
    }
    return Allocator_Error_None;
}

Allocator_Error
string_append_string(String_Builder *builder, String text)
{
    // Nothing to do. Also avoid undefined behavior with `memcpy`.
    if (text.data == NULL || text.len == 0)
        return Allocator_Error_None;

    Allocator_Error err = _string_builder_check_resize(builder, text.len);
    if (err)
        return err;

    memcpy(&builder->buffer[builder->len], text.data, text.len);
    builder->len += text.len;
    return Allocator_Error_None;
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
    // Nothing to do. Also avoid undefined behavior with `memcpy`.
    if (text.data == NULL || text.len == 0)
        return Allocator_Error_None;

    // Move the old text to the new location.
    // e.g. given "hi mom!" (len = 7), prepend "yay " (len = 4)
    // 1.   Resize builder to be "hi mom!\0\0\0\0" (len = 11)
    //      Note that the new region is zeroed out.
    Allocator_Error err = _string_builder_check_resize(builder, text.len);
    if (err)
        return err;

    // 2.   Move old text "hi mom!" to new location (4): "hi mhi mom!"
    //      We use memmove because these regions definitely overlap.
    memmove(&builder->buffer[text.len], &builder->buffer[0], builder->len);

    // 3.   Copy new text to old location (0): "yay hi mom!"
    memcpy(&builder->buffer[0], text.data, text.len);
    builder->len += text.len;
    return Allocator_Error_None;
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
    // We should be nul-terminated long before we reach this point.
    assert(builder->buffer[builder->len] == 0);
    return builder->buffer;
}
