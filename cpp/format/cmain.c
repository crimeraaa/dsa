#include <limits.h>
#include <stdio.h>
#include "parse.h"

void test_signed_integers(void)
{
    write_stream(stdout, "CHAR_BIT = %i\n", CHAR_BIT);
    write_stream(stdout, "CHAR_MIN = %i\n", CHAR_MIN);
    write_stream(stdout, "CHAR_MAX = %i\n", CHAR_MAX);
    write_stream(stdout, "SHRT_MIN = %i\n", SHRT_MIN);
    write_stream(stdout, "SHRT_MAX = %i\n", SHRT_MAX);
    write_stream(stdout, "INT_MIN = %i\n", INT_MIN); // -2,147,483,648
    write_stream(stdout, "INT_MAX = %i\n", INT_MAX); // +2,147,483,648
    // The `LONG_*` and `LONG_*` macros are already literals of the proper type
    // due to the 'L' suffix. Same thing with `LLONG_*` and the "LL" suffix.
    write_stream(stdout, "LONG_MIN = %li\n", LONG_MIN);
    write_stream(stdout, "LONG_MAX = %li\n", LONG_MAX);
    write_stream(stdout, "LLONG_MIN = %lli\n", LLONG_MIN);
    write_stream(stdout, "LLONG_MAX = %lli\n", LLONG_MAX);
}

void test_unsigned_integers(void)
{
    write_stream(stdout, "UCHAR_MAX = %u\n", UCHAR_MAX);
    write_stream(stdout, "USHRT_MAX = %u\n", USHRT_MAX);
    write_stream(stdout, "UINT_MAX = %u\n", UINT_MAX);
    write_stream(stdout, "ULONG_MAX = %lu\n", ULONG_MAX);
    write_stream(stdout, "ULLONG_MAX = %llu\n", ULLONG_MAX);
}

int main(void)
{
    write_stream(stdout, "Hi mom!\n");
    write_stream(stdout, "My favorite number is %d. %i/%d!\n", 7, 7, 11);
    test_signed_integers();
    test_unsigned_integers();
    return 0;
}
