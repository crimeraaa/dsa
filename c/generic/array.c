#include "array.h"

#define alpha(x)                    #x
#define bravo(x)                    alpha(x)
#define loginfo(msg) __FILE__ ":"   bravo(__LINE__) ": " msg
#define logformat(func, fmts)       loginfo(func ": ") fmts
#define logprint(func, fmts, ...)   fprintf(stderr, logformat(func, fmts), __VA_ARGS__)

ga_array ga_init(size_t count, const ti_typeinfo *info)
{
    ga_array inst = {
        .rawbytes = (count != 0) ? malloc(info->size * count) : nullptr,
        .count = 0, // No elements written yet.
        .capacity = count, // Can store this many elements.
        .info = info
    };
    if (count != 0 && inst.rawbytes == nullptr) {
        perror(loginfo("aaa"));
    }
    return inst;
}

void ga_deinit(ga_array *self)
{
    if (!self) {
        return;
    }
    free(self->rawbytes);
}

void *ga_assign(ga_array *self, void *dst, const void *src)
{
    #define cursed_cast(T) *(T*)dst = *(T*)src; return dst
    // Hacky fundamental type lookup
    switch (self->info->spec)
    {
        case 'c': switch (self->info->length)
        {
            case TI_LENGTH_NONE:    cursed_cast(char);
            case TI_LENGTH_LONG:    cursed_cast(wchar_t);
            default:                goto endfn; // Avoid fall-throughs here.
        }
        case 'd': case 'i': switch (self->info->length)
        {
            case TI_LENGTH_NONE:    cursed_cast(int);
            case TI_LENGTH_LONG:    cursed_cast(long);
            case TI_LENGTH_LLONG:   cursed_cast(long long);
            case TI_LENGTH_SHORT:   cursed_cast(short);
            case TI_LENGTH_SSHORT:  cursed_cast(signed char);
            default:                goto endfn;
        }
        case 'u': switch (self->info->length)
        {
            case TI_LENGTH_NONE:    cursed_cast(unsigned int);
            case TI_LENGTH_LONG:    cursed_cast(unsigned long);
            case TI_LENGTH_LLONG:   cursed_cast(unsigned long long);
            case TI_LENGTH_SHORT:   cursed_cast(unsigned short);
            case TI_LENGTH_SSHORT:  cursed_cast(unsigned char);
            case TI_LENGTH_SIZE_T:  cursed_cast(size_t);
            default:                goto endfn;
        }
        case 'f': switch (self->info->length)
        {
            case TI_LENGTH_NONE:    cursed_cast(float);
            case TI_LENGTH_LONG:    cursed_cast(double);
            case TI_LENGTH_LLONG:   cursed_cast(long double);
            default:                goto endfn;
        }
        // If you pass string literals, I can only say good luck!
        // The only way this WON'T crash is if they're heap allocated.
        // `ga_t` does not make any heap-allocations for individual items!
        case 's': switch (self->info->length)
        {
            case TI_LENGTH_NONE:    cursed_cast(char *);
            case TI_LENGTH_LONG:    cursed_cast(wchar_t *);
            default:                goto endfn;
        }
        case 'p': cursed_cast(void *);
    }
endfn:
    return nullptr;
    #undef cursed_cast
}

static inline size_t grow_buffer(size_t capacity)
{
    return (capacity > 0) ? (capacity * 2) : 16; // TODO: Use named constant
}

bool ga_push_back(ga_array *self, const void *src)
{
    // Reached current allocated limit so try to resize buffer.
    if (self->count + 1 > self->capacity) {
        self->capacity = grow_buffer(self->capacity);
        ga_byte *tmp = realloc(self->rawbytes, self->info->size * self->capacity);
        if (tmp == nullptr) {
            perror(loginfo("ga_push_back(): failed to resize buffer"));
            return false;
        }
        self->rawbytes = tmp; // realloc already frees original memory if OK.
    }
    void *dst = &self->rawbytes[self->info->size * self->count];
    bool success = (ga_assign(self, dst, src) != nullptr);
    if (success) {
        self->count++;
    }
    return success;
}

void *ga_retrieve(const ga_array *self, size_t index)
{
    if (index < self->capacity) {
        size_t rawoffset = self->info->size * index;
        return &self->rawbytes[rawoffset];
    }
    logprint("ga_retrieve()", "Invalid index %zu (of %zu)", index, self->capacity);
    return nullptr;
}
