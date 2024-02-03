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
        "};\n",
        (void*)self->rawbytes,
        self->count,
        self->capacity);
    printf("Total usage: %zu bytes\n", self->info->size * self->capacity);
    for (size_t i = 0; i < self->count; i++) {
        // Because ga_TypeInfo is only available at runtime, no way we can
        // emulate the C++11 keyword `auto`.
        const int *pi = ga_retrieve(self, i);
        printf("self[%zu] = %i (& = %p)\n", i, (pi) ? *pi : -1, (void*)pi); 
    }
}

int main(void)
{
    // Order of operations for `&`: https://stackoverflow.com/a/40167118
    ga_array ga = ga_init(0, &(ga_fundtypes.i[GA_TYPELENGTH_NONE]));
    ga_array *p = &ga;
    int i = 13;
    int ii = 45;
    ga_push_back(p, &i);
    ga_push_back(p, &ii);
    ga_print(p);
    ga_deinit(p);
    (void)ga;
}
