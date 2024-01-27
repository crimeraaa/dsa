#include <cstdio>
#include <cstdlib>

#include "initializer.h"

static void test_deinit()
{
    printf("Hello from after main!\n");
}

program_init(test_init)
{
    printf("Hello from before main!\n");
    atexit(test_deinit);
}

int main()
{
    return 0;
}
