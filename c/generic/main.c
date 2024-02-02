#include <stddef.h>
#include <stdio.h>
#include "ga_array.h"

void ga_print(const ga_array *self)
{
    printf(
        "*self = {\n"
        "   rawbytes = %p\n"
        "   count = %zu\n"
        "   capacity = %zu\n"
        "   size = %zu\n"
        "};\n",
        (void*)self->rawbytes,
        self->count,
        self->capacity,
        self->size);
    printf("Total usage: %zu bytes\n", self->size * self->capacity);
}

int main(void)
{
    ga_array ga = ga_init(16, sizeof(int));
    ga_print(&ga);
    ga_deinit(&ga);
    (void)ga;
}
