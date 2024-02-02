#include "ga_array.h"

#define alpha(x) #x
#define bravo(x) alpha(x)
#define logmsg(msg) __FILE__ ":" bravo(__LINE__) ": " msg

ga_array ga_init(size_t count, size_t size)
{
    ga_array inst = {
        .rawbytes = (count != 0) ? malloc(size * count) : nullptr,
        .count = 0, // No elements written yet.
        .capacity = count, // Can store this many elements.
        .size = size,
        .info = {
            .length = GA_TYPE_LENGTH_NONE, 
            .type = {.p = nullptr},
            .spec = '\0',
            .sign = false,
        }
    };
    if (count != 0 && inst.rawbytes == nullptr) {
        perror(logmsg("aaa"));
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

    switch (self->info.spec)
    {
        case 'c': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_cast(char);
            case GA_TYPE_LENGTH_LONG:   cursed_cast(wchar_t);
            default:                    goto endfn; // Avoid fall-throughs here.
        }
        case 'd': case 'i': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_cast(int);
            case GA_TYPE_LENGTH_LONG:   cursed_cast(long);
            case GA_TYPE_LENGTH_LLONG:  cursed_cast(long long);
            case GA_TYPE_LENGTH_SHORT:  cursed_cast(short);
            case GA_TYPE_LENGTH_SSHORT: cursed_cast(signed char);
            default:                    goto endfn;
        }
        case 'u': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_cast(unsigned int);
            case GA_TYPE_LENGTH_LONG:   cursed_cast(unsigned long);
            case GA_TYPE_LENGTH_LLONG:  cursed_cast(unsigned long long);
            case GA_TYPE_LENGTH_SHORT:  cursed_cast(unsigned short);
            case GA_TYPE_LENGTH_SSHORT: cursed_cast(unsigned char);
            case GA_TYPE_LENGTH_SIZE_T: cursed_cast(size_t);
            default:                    goto endfn;
        }
        case 'f': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_cast(float);
            case GA_TYPE_LENGTH_LONG:   cursed_cast(double);
            case GA_TYPE_LENGTH_LLONG:  cursed_cast(long double);
            default:                    goto endfn;
        }
        // If you pass string literals, I can only say good luck!
        // The only way this WON'T crash is if they're heap allocated.
        // `ga_array` does not make any heap-allocations for individual items!
        case 's': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_cast(char *);
            case GA_TYPE_LENGTH_LONG:   cursed_cast(wchar_t *);
            default:                    goto endfn;
        }
        case 'p': cursed_cast(void *);
    }
endfn:
    return nullptr;
    #undef cursed_cast
}

bool ga_push_back(ga_array *self, const void *src)
{
    void *dst = &self->rawbytes[self->size * self->count];
    bool success = (ga_assign(self, dst, src) != nullptr);
    if (success) {
        self->count++;
    }
    return success;
}

void *ga_retrieve(const ga_array *self, size_t index)
{
    #define cursed_return(T) \
        return &self->rawbytes[sizeof(T) * index]

    if (index >= self->capacity) {
        return nullptr;
    }

    /**
     * Presenting: "Ghetto C++ w/ Macros" (courtesy of a Tsoding Daily comment)
     */
    switch (self->info.spec)
    {
        case 'c': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_return(char);
            case GA_TYPE_LENGTH_LONG:   cursed_return(wchar_t);
            default:                    goto endfn; // Avoid fall-throughs here.
        }
        case 'd': case 'i': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_return(int);
            case GA_TYPE_LENGTH_LONG:   cursed_return(long);
            case GA_TYPE_LENGTH_LLONG:  cursed_return(long long);
            case GA_TYPE_LENGTH_SHORT:  cursed_return(short);
            case GA_TYPE_LENGTH_SSHORT: cursed_return(signed char);
            default:                    goto endfn;
        }
        case 'u': switch (self->info.length) 
        {
            case GA_TYPE_LENGTH_NONE:   cursed_return(unsigned int);
            case GA_TYPE_LENGTH_LONG:   cursed_return(unsigned long);
            case GA_TYPE_LENGTH_LLONG:  cursed_return(unsigned long long);
            case GA_TYPE_LENGTH_SHORT:  cursed_return(unsigned short);
            case GA_TYPE_LENGTH_SSHORT: cursed_return(unsigned char);
            case GA_TYPE_LENGTH_SIZE_T: cursed_return(size_t);
            default:                    goto endfn;
        }
        case 'f': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_return(float);
            case GA_TYPE_LENGTH_LONG:   cursed_return(double);
            case GA_TYPE_LENGTH_LLONG:  cursed_return(long double);
            default:                    goto endfn;
        }
        case 's': switch (self->info.length)
        {
            case GA_TYPE_LENGTH_NONE:   cursed_return(char *);
            case GA_TYPE_LENGTH_LONG:   cursed_return(wchar_t *);
            default:                    goto endfn;
        }
        case 'p': cursed_return(void *);
    }    
endfn:
    return nullptr;
    #undef cursed_return
}
