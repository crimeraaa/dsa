#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#include "parse.h"

void test_signed_integers(void)
{
    print_format("CHAR_BIT = %i\n", CHAR_BIT);
    print_format("CHAR_MIN = %i\n", CHAR_MIN);
    print_format("CHAR_MAX = %i\n", CHAR_MAX);
    print_format("SHRT_MIN = %i\n", SHRT_MIN);
    print_format("SHRT_MAX = %i\n", SHRT_MAX);
    print_format("INT_MIN = %i\n", INT_MIN); // -2,147,483,648
    print_format("INT_MAX = %i\n", INT_MAX); // +2,147,483,648
    // The `LONG_*` and `LONG_*` macros are already literals of the proper type
    // due to the 'L' suffix. Same thing with `LLONG_*` and the "LL" suffix.
    print_format("LONG_MIN = %li\n", LONG_MIN);
    print_format("LONG_MAX = %li\n", LONG_MAX);
    print_format("LLONG_MIN = %lli\n", LLONG_MIN);
    print_format("LLONG_MAX = %lli\n", LLONG_MAX);
}

void test_unsigned_integers(void)
{
    print_format("UCHAR_MAX = %u\n", UCHAR_MAX);
    print_format("USHRT_MAX = %u\n", USHRT_MAX);
    print_format("UINT_MAX = %u\n", UINT_MAX);
    print_format("ULONG_MAX = %lu\n", ULONG_MAX);
    print_format("ULLONG_MAX = %llu\n", ULLONG_MAX);
}

void test_chars(void)
{
    char ch = 'A';
    wchar_t wch = L'B';
    print_format("char ch = '%c';\n", ch);
    print_format("wchar_t wch = L'%lc';\n", wch);
}

/**
 * These won't work unless the stream is wide-oriented.
 * Also this means we'll need a version of `print_format` for wide characters.
 * 
 * Strangely enough `printf` in normal orientation prints `wchar_t*` just fine.
 */
void test_wchars(void)
{
    wchar_t lc = L'X';
    const wchar_t *ls = L"Hi mom!";
    // U+00A1: Iverted Exclamation Point, U+00F1: Latin Small Letter N with Tilde
    const wchar_t *spanish = L"\u00A1Hola Se\u00F1or y Se\u00F1orita!";
    print_format("'%lc'\n", lc);
    print_format("\"%ls\"\n", ls);
    print_format("\"%ls\"\n", spanish);
}

int main(void)
{
    if (setlocale(LC_CTYPE, "") == NULL) {
        perror("Failed to set locale to all");
        return 1;
    }
    // Positive if wide-char oriented, negative if not, 0 if no orientation.
    // fputwc('\0', stdout);
    print_format("Hi mom!\n");
    print_format("My favorite number is %d. %i/%d!\n", 7, 7, 11);
    test_signed_integers();
    test_unsigned_integers();
    test_chars();
    test_wchars();
    return 0;
}
