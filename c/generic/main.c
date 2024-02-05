#include <stddef.h>
#include <stdio.h>
#include "array.h"

typedef struct TI_Test {
    char key;
    int value;
} TI_Test;

static inline TI_Test *ttest_init(TI_Test *self)
{
    // Remember NULL/nullptr (usually) is 0, and 0 in C is false.
    if (self) {
        self->key = 0;
        self->value = 0;
    }
    return self;
}

TI_Test *ttest_copy(TI_Test *self, const TI_Test *other)
{
    self->key = other->key;
    self->value = other->value;
    return self;
}

TI_Test *ttest_move(TI_Test *self, TI_Test *other)
{
    self->key = other->key;
    self->value = other->value;
    other->key = 0;
    other->value = 0;
    return self;
}

void ttest_deinit(TI_Test *self)
{
    self->key = 0;
    self->value = 0;
}

/** 
 * These casts are allowed because the internal function pointers are called
 * only by implementation functions, which in turn only call them in the forms:
 * ```cpp
 * void self->info->fnlist->init(void *dst);
 * void self->info->fnlist->copy(void *dst, const void *src);
 * void self->info->fnlist->move(void *dst, void *src);
 * void self->info->fnlist->deinit(void *dst);
 * ```
 */
const ti_typefns ttest_fns = {
    .init   = (ti_initfn*)   &ttest_init,
    .copy   = (ti_copyfn*)   &ttest_copy,
    .move   = (ti_movefn*)   &ttest_move,
    .deinit = (ti_deinitfn*) &ttest_deinit,
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

void test_TI_Test_array(void)
{
    ga_array ga = ga_init(0, &ttest_fns);
}

int main(void)
{
    // Order of operations for `&`: https://stackoverflow.com/a/40167118
    ga_array ga = ga_init(0, ti_query('i', TI_LENGTH_NONE));
    ga_array *p = &ga;
    for (int i = 0; i < 16; i++) {
        int x = rand() % 0xFF;
        ga_push_back(p, &x);
    }
    ga_print(p);
    ga_deinit(p);
    (void)ga;
    test_integer_ti();
    test_pointer_ti();
    return 0;
}
