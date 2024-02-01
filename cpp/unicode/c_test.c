#include <stdio.h>
#include <wchar.h>
#include "utf8.h"

/* Use `static inline` else we get a linker error. */
int stream_iswide(FILE *stream)
{
    return fwide(stream, 0) > 0;
}

int stream_isnarrow(FILE *stream)
{
    return fwide(stream, 0) < 0;
}

const char *stream_orientation(FILE *stream)
{
    return stream_iswide(stream) ? "wide" 
        : stream_isnarrow(stream) ? "narrow" : "none";
}

int gen_fputc(char c, FILE *stream)
{
    return stream_iswide(stream) ? wide_fputc(c, stream) : fputc(c, stream);
}

int gen_fputs(const char *s, FILE *stream)
{
    // fwide(stream, 0) returns the current orientation of `stream`.
    // - positive indicates it's wide-oriented.
    // - negative indicates it's narrow-oriented.
    // - 0 indicates it's not set. We'll default to narrow for simplicity.
    return stream_iswide(stream) 
        ? wide_fputs(s, stream) 
        : fputs(s, stream);
}

int gen_fputwc(int wc, FILE *stream)
{
    return stream_isnarrow(stream) 
        ? narrow_fputwc(wc, stream) 
        : wide_fputc(wc, stream); // Use wrapper for return type consistency.
}

int gen_fputws(const wchar_t *ws, FILE *stream)
{
    return stream_isnarrow(stream) 
        ? narrow_fputws(ws, stream) 
        : fputws(ws, stream); // both `fputs` and `fputws` return `int`.
}

int gen_putc(int c)
{
    return gen_fputc(c, stdout);
}

int gen_puts(const char *s)
{
    return gen_fputs(s, stdout);
}

int gen_putwc(int wc)
{
    return gen_fputwc(wc, stdout);
}

int gen_putws(const wchar_t *ws)
{
    return gen_fputws(ws, stdout);
}

int main(void)
{
    wide_fputs("Hello there!\n", stdout);
    gen_puts(stream_orientation(stdout));
    gen_putc('\n');
    gen_putws(L"Hi mom!\n");
    gen_putwc(L'X');
    gen_putwc(L'\n');
    // narrow_fputws(L"Hello there!\n", stdout);
    // When not yet called `setlocale(LC_CTYPE, "");` this should fail
    // narrow_fputws(L"\u00A1Hola Se\u00F1or y Se\u00F1orita!", stdout);
    return 0;
}
