#include <stdio.h>

typedef void *initfn(void *self);
typedef void *copyfn(void *self, const void *other);
typedef void *movefn(void *self, void *other);
typedef void deinitfn(void *self);

#define stringify_(expanded)   #expanded 
#define stringify(toexpand)    stringify_(toexpand)
#define logformat()             __FILE__ ":" stringify(__LINE__) ": "
#define print_func()            fprintf(stderr, logformat() "%s()\n", __func__);

typedef struct sample_t {
    char key;
    int value;
} sample_t;

void st_print(const sample_t *self)
{
    const void *p = self; // Raw memory address
    printf("\t%p: {.key = %c, .value = %i}\n", p, self->key, self->value);
}

sample_t *st_init(sample_t *self)
{
    if (self) {
        self->key = '0';
        self->value = 0;
    }
    print_func();
    st_print(self);
    return self;
}

sample_t *st_copy(sample_t *self, const sample_t *other)
{
    print_func();
    st_print(self);
    st_print(other);
    self->key = other->key;
    self->value = other->value;
    return self;
}

sample_t *st_move(sample_t *self, sample_t *other)
{
    print_func();
    st_print(self);
    st_print(other);
    self = st_copy(self, other);
    other = st_init(other);
    return self;
}

void st_deinit(sample_t *self)
{
    self = st_init(self);
    print_func();
    st_print(self);
}

int main(void)
{
    initfn *init = (initfn*)&st_init; // Terrible!!!
    copyfn *copy = (copyfn*)&st_copy;
    movefn *move = (movefn*)&st_move;
    deinitfn *deinit = (deinitfn*)&st_deinit;
    sample_t s1, s2;
    init(&s1);
    s2.key = 'c';
    s2.value = 13;
    copy(&s1, &s2);
    move(&s2, &s1);
    deinit(&s1);
    deinit(&s2);
    return 0;
}
