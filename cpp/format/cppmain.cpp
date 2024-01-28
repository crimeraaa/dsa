#include <cstdio>
#include "parse.h"

void test_literals()
{
    print_format("Hi %s!\n", "mom");
    print_format("Turn oven to %i degrees.\n", 350);
}

void test_variables()
{
    char ch = 'H';
    int n = -21, i = 13;
    unsigned ui = 14, uo = 0;
    const char *s = "Hi mom!";
    print_format("char ch = '%c';\nint n = %i, i = %d;\n", ch, n, i);
    print_format("unsigned ui = %u, uo = %u;\n", ui, uo);
    print_format("const char *s = \"%s\";\n", s);
}

int main()
{
    test_literals();
    test_variables();
    return 0;
}
