#include <limits.h>
#include <stdio.h>
#include "parse.h"

void test_limits(void)
{
    write_format(stdout, "CHAR_BIT = %i\n", CHAR_BIT);
    write_format(stdout, "CHAR_MAX = %i\n", CHAR_MAX);
    write_format(stdout, "CHAR_MIN = %i\n", CHAR_MIN);
    write_format(stdout, "INT_MIN = %i\n", INT_MIN); // -2,147,483,648
    write_format(stdout, "INT_MAX = %i\n", INT_MAX); // +2,147,483,648
}

int main(void)
{
    write_format(stdout, "Hi mom!\n");
    write_format(stdout, "My favorite number is %d. %i/%d\n", 7, 7, 11);
    test_limits();
    return 0;
}
