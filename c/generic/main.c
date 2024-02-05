#include <stddef.h>
#include <stdio.h>
#include "array.h"

typedef struct sample_t {
    char key;
    int value;
} sample_t;

void st_print(const sample_t *self)
{
    const void *p = self;
    printf("\t%p: {.key = %c, .value = %i}\n", p, self->key, self->value);
}

sample_t *st_init(sample_t *self)
{
    // Remember NULL/nullptr (usually) is 0, and 0 in C is false.
    if (self) {
        self->key = 0;
        self->value = 0;
    }
    return self;
}

sample_t *st_copy(sample_t *self, const sample_t *other)
{
    self->key = other->key;
    self->value = other->value;
    return self;
}

sample_t *st_move(sample_t *self, sample_t *other)
{
    self = st_copy(self, other);
    other = st_init(other);
    return self;
}

void st_deinit(sample_t *self)
{
    self = st_init(self);
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
const ti_typefns st_fns = {
    .init   = (ti_initfn*)   &st_init,
    .copy   = (ti_copyfn*)   &st_copy,
    .move   = (ti_movefn*)   &st_move,
    .deinit = (ti_deinitfn*) &st_deinit,
};

const ti_typeinfo st_info = {
    .size = sizeof(sample_t),
    .fnlist = &st_fns,
    .length = TI_LENGTH_NONE,
    .spec = '0',
    .is_signed = false,
    .is_fundamental = false,
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

void st_array(void)
{
    ga_array ga = ga_init(0, &st_info);
    ga_array *self = &ga;
    for (int i = 0; i < 16; i++) {
        // Getting the addresses of literals was introduced in C11 I believe.
        ga_push_back(self, &(sample_t){
            .key = 'a' + i, 
            .value = rand() % 0xFF
        });
    }
    for (size_t i = 0; i < self->count; i++) {
        st_print(ga_rawretrieve(self, i));
    }
    ga_deinit(self);
}

/* More accurately, copying exact pointers over. */
void string_array(void)
{
    static const char *strings[] = {
        "Hi mom!",
        "Hello there.",
        "The quick brown fox jumps over the lazy dog",
        "She sells sea shells by the seashore..."
    };
    static const size_t count = sizeof(strings) / sizeof(strings[0]);
    // 1D Array of pointers to the pointers in `strings`.
    ga_array ga = ga_init(count, ti_query('s', TI_LENGTH_NONE));
    ga_array *self = &ga;
    for (size_t i = 0; i < count; i++) {
        // Just copying pointers to readonly stack-allocated strings.
        ga_push_back(self, &strings[i]); 
    }
    for (size_t i = 0; i < self->count; i++) {
        const char **string = ga_retrieve(self, i);
        printf("self[%zu]: \"%s\"\n", i, *string);
    }
    printf("Usage: %zu bytes.\n", self->info->size * self->capacity);
    ga_deinit(self);
}

int main(void)
{
    // Order of operations for `&`: https://stackoverflow.com/a/40167118
    ga_array ga = ga_init(0, ti_query('i', TI_LENGTH_NONE));
    ga_array *p = &ga;
    for (int i = 0; i < 16; i++) {
        // This address-of-literal syntax was introduced in C11.
        ga_push_back(p, &(int){rand() % 0xFF});
    }
    ga_print(p);
    ga_deinit(p);
    test_integer_ti();
    test_pointer_ti();
    st_array();
    string_array();
    return 0;
}
