#include <cstdio>

#include "initializer.h"

/**
 * GNU C extension for a function to run before main.
 * You could also pass a void function to `atexit` if you'd like, as that's
 * more standard.
*/
void init() __attribute__((constructor));

/**
 * GNU C extension for a function to run after main or `exit` is called.
*/
void deinit() __attribute__((destructor));

void init()
{
    printf("Hello from before main!\n");
}

void deinit()
{
    printf("Hello from after main!\n");
}

int main()
{
    printf("Hello from inside of main!\n");
    return 0;
}
