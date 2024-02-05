#include "input.h"

char *get_string(const char *fmts, ...)
{
    va_list args;
    va_start(args, fmts);
    vfprintf(stdout, fmts, args);
    va_end(args);
    return readline(stdin);    
}

char *readline(FILE *stream)
{
    char writer[32] = {0}; // Constantly overwritten by `fgets`.
    char *buffer = NULL;   // Append contents of `writer` to here.
    static const size_t maximum = sizeof(writer); // Not including nul char.
    size_t count = 0; // Number of characters currently written to `buffer`.
    size_t capacity = maximum; // Total `char`'s allocated for `buffer`.
    // man 3 fgets: Returns NULL on error on end on of file when no chars read.
    //              It also inserts a nul char at the end.
    while (fgets(writer, maximum, stream)) {
        char *dummy = realloc(buffer, capacity); // Try to resize buffer.
        if (!dummy) {
            free(buffer);
            buffer = nullptr;
            logperror("get_string", "realloc failed");
            break;
        }
        buffer = dummy;
        // Append contents of `writer` into `buffer`.
        for (size_t i = 0; i < strcspn(writer, "\r\n"); i++) {
            buffer[count++] = writer[i];
        }
        buffer[count] = 0;
        // maximum - 1 indicates we probably truncated the input.
        // So if we haven't reached it, it must mean no truncation was done
        // and we can safely return `buffer` as is.
        if (bd_strlen(writer, maximum) != (maximum - 1)) {
            break;
        }
        capacity += (capacity << 1); // Approximate 1.5x growth factor
    }
    return buffer;
}

size_t bd_strlen(const char *subject, size_t maximum)
{
    // Try to find a pointer to the end of string, else `NULL`.
    const char *nulchar = memchr(subject, '\0', maximum);
    // Return the resulting string length, or `maximum` as a fallback.
    return (nulchar) ? (size_t)(nulchar - subject) : maximum; 
}
