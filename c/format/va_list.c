#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *csformat(const char *fmts, ...)
{
    char *writer = NULL;
    int length = 0;
    va_list args, copy;
    
    va_start(args, fmts);
    va_copy(copy, args);

    length = vsnprintf(NULL, 0, fmts, args);
    if (length < 0) {
        goto cleanup;
    }
    length++; // add 1 for nul char as it wasn't counted
    writer = malloc(sizeof(char) * length);
    if (writer == NULL) {
        goto cleanup;
    }
    length = vsnprintf(writer, length, fmts, copy);
    if (length < 0) {
        free(writer);
        writer = NULL;
        goto cleanup;
    }
cleanup:
    va_end(args);
    va_end(copy);
    return writer;
}
