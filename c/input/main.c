#include "input.h"

int main(void)
{
    char *name = get_string("Enter your name: ");
    char *food = get_string("Hi %s! What's your favorite food? ", name);
    printf("name = %s, food = %s\n", name, food);
    free(name);
    free(food);
    return 0;
}
