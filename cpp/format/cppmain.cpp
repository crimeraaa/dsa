#include <cstdio>
#include "parse.h"

int main()
{
    char ch = 'H';
    int n = -21, i = 13;
    unsigned ui = 14, uo = 0;
    write_format(stdout, "char ch = '%c';\nint n = %i, i = %d;\n", ch, n, i);
    write_format(stdout, "unsigned ui = %u, uo = %u;\n", ui, uo);
    write_format(stdout, "Hi %s!\n", "mom");
    write_format(stdout, "Turn oven to %i degrees.\n", 350);
    return 0;
}
