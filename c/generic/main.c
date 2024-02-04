#include <stddef.h>
#include <stdio.h>
#include "array.h"

struct TI_Test {
    char key;
    int value;
};

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
        // Because ti_typeinfo is only available at runtime, no way we can
        // emulate the C++11 keyword `auto`.
        const int *pi = ga_retrieve(self, i);
        printf("self[%zu] = %i (& = %p)\n", i, (pi) ? *pi : -1, (void*)pi); 
    }
}

void test_integer_ti(void)
{
    const ti_typeinfo *ti = ti_query('i', TI_LENGTH_NONE);
    int i = 21, ii = 49;
    printf("before: i = %i, ii = %i\n", i, ii);
    ti->fnlist->move(&i, &ii);
    printf("after: i = %i, ii = %i\n", i, ii);
}

void test_pointer_ti(void)
{
    const ti_typeinfo *ti = ti_query('p', TI_LENGTH_NONE);
    void *p;
    ti->fnlist->init(&p);
    printf("before: p = %p\n", p);
    ti->fnlist->copy(&p, &ti);
    printf("after: p = %p\n", p);
}

int main(void)
{
    // Order of operations for `&`: https://stackoverflow.com/a/40167118
    ga_array ga = ga_init(0, ti_query('i', TI_LENGTH_NONE));
    ga_array *p = &ga;
    int i = 13;
    int ii = 45;
    ga_push_back(p, &i);
    ga_push_back(p, &ii);
    ga_print(p);
    ga_deinit(p);
    (void)ga;
    test_integer_ti();
    test_pointer_ti();
    return 0;
}
