#include <cstdio>
#include "parse.h"

void test_literals()
{
    write_stream(stdout, "Hi %s!\n", "mom");
    write_stream(stdout, "Turn oven to %i degrees.\n", 350);
}

void test_variables()
{
    char ch = 'H';
    int n = -21, i = 13;
    unsigned ui = 14, uo = 0;
    const char *s = "Hi mom!";
    write_stream(stdout, "char ch = '%c';\nint n = %i, i = %d;\n", ch, n, i);
    write_stream(stdout, "unsigned ui = %u, uo = %u;\n", ui, uo);
    write_stream(stdout, "const char *s = \"%s\";\n", s);
}

int main()
{
    test_literals();
    test_variables();
    return 0;
}
