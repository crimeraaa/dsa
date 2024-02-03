#include "ga_array.h"

#define alpha(x) #x
#define bravo(x) alpha(x)
#define logmsg(msg) __FILE__ ":" bravo(__LINE__) ": " msg

/**
 * BEGIN:   MACRO HELL
  */

#define make_typeinfo_fund(LUT_Key, C_Type, C_Spec, Is_Signed) \
    [LUT_Key] = {  \
        .size = sizeof(C_Type), \
        .length = LUT_Key, \
        .spec = C_Spec, \
        .sign = Is_Signed, \
        .fund = true, \
        .copy = nullptr, \
    }

#define make_typeinfo_invalid(K, T) \
    make_typeinfo_fund(K, T, '\0', false)

#define make_typeinfo_i(K, T) \
    make_typeinfo_fund(K, signed T, 'i', true)

#define make_typeinfo_u(K, T) \
    make_typeinfo_fund(K, unsigned T, 'u', false)

/* Not just for int, works for all integral types. */
#define make_typeinfo_integral(macrofn) \
    macrofn(GA_TYPELENGTH_NONE, int), \
    macrofn(GA_TYPELENGTH_LONG, long), \
    macrofn(GA_TYPELENGTH_LLONG, long long), \
    macrofn(GA_TYPELENGTH_SHORT, short), \
    macrofn(GA_TYPELENGTH_SSHORT, char),

/**
 * END:     MACRO HELL
 */

const ga_fundtype_lookup ga_fundtypes = {
    .i = {
        make_typeinfo_integral(make_typeinfo_i)
        make_typeinfo_invalid(GA_TYPELENGTH_SIZE_T, size_t), // No signed size_t
    },
    .u = {
        make_typeinfo_integral(make_typeinfo_u)
        make_typeinfo_fund(GA_TYPELENGTH_SIZE_T, size_t, 'u', false),
    },
    // Actually, most string operations don't care about signedness anyway.
    .c = {
        make_typeinfo_fund(GA_TYPELENGTH_NONE, char, 'c', (CHAR_MIN == SCHAR_MIN)),
        make_typeinfo_fund(GA_TYPELENGTH_LONG, wchar_t, 'c', (WCHAR_MIN != 0)),
    },
    .s = {
        make_typeinfo_fund(GA_TYPELENGTH_NONE, char *, 's', (CHAR_MIN == SCHAR_MIN)),
        make_typeinfo_fund(GA_TYPELENGTH_LONG, wchar_t *, 's', (WCHAR_MIN != 0)),
    },
};

const ga_TypeInfo *ga_query_fundtypes(char key, enum ga_TypeLength len)
{
    (void)key; (void)len;
    return nullptr;
}

ga_array ga_init(size_t count, const ga_TypeInfo *info)
{
    ga_array inst = {
        .rawbytes = (count != 0) ? malloc(info->size * count) : nullptr,
        .count = 0, // No elements written yet.
        .capacity = count, // Can store this many elements.
        .info = info
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
    // Hacky fundamental type lookup
    switch (self->info->spec)
    {
        case 'c': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_cast(char);
            case GA_TYPELENGTH_LONG:    cursed_cast(wchar_t);
            default:                    goto endfn; // Avoid fall-throughs here.
        }
        case 'd': case 'i': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_cast(int);
            case GA_TYPELENGTH_LONG:    cursed_cast(long);
            case GA_TYPELENGTH_LLONG:   cursed_cast(long long);
            case GA_TYPELENGTH_SHORT:   cursed_cast(short);
            case GA_TYPELENGTH_SSHORT:  cursed_cast(signed char);
            default:                    goto endfn;
        }
        case 'u': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_cast(unsigned int);
            case GA_TYPELENGTH_LONG:    cursed_cast(unsigned long);
            case GA_TYPELENGTH_LLONG:   cursed_cast(unsigned long long);
            case GA_TYPELENGTH_SHORT:   cursed_cast(unsigned short);
            case GA_TYPELENGTH_SSHORT:  cursed_cast(unsigned char);
            case GA_TYPELENGTH_SIZE_T:  cursed_cast(size_t);
            default:                    goto endfn;
        }
        case 'f': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_cast(float);
            case GA_TYPELENGTH_LONG:    cursed_cast(double);
            case GA_TYPELENGTH_LLONG:   cursed_cast(long double);
            default:                    goto endfn;
        }
        // If you pass string literals, I can only say good luck!
        // The only way this WON'T crash is if they're heap allocated.
        // `ga_array` does not make any heap-allocations for individual items!
        case 's': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_cast(char *);
            case GA_TYPELENGTH_LONG:    cursed_cast(wchar_t *);
            default:                    goto endfn;
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
            perror("ga_push_back(): failed to resize buffer");
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
    #define cursed_return(T) \
        return &self->rawbytes[sizeof(T) * index]

    if (index >= self->capacity) {
        return nullptr;
    }

    /**
     * Presenting: "Ghetto C++ w/ Macros" (courtesy of a Tsoding Daily comment)
     */
    switch (self->info->spec)
    {
        case 'c': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_return(char);
            case GA_TYPELENGTH_LONG:    cursed_return(wchar_t);
            default:                    goto endfn; // Avoid fall-throughs here.
        }
        case 'd': case 'i': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_return(int);
            case GA_TYPELENGTH_LONG:    cursed_return(long);
            case GA_TYPELENGTH_LLONG:   cursed_return(long long);
            case GA_TYPELENGTH_SHORT:   cursed_return(short);
            case GA_TYPELENGTH_SSHORT:  cursed_return(signed char);
            default:                    goto endfn;
        }
        case 'u': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_return(unsigned int);
            case GA_TYPELENGTH_LONG:    cursed_return(unsigned long);
            case GA_TYPELENGTH_LLONG:   cursed_return(unsigned long long);
            case GA_TYPELENGTH_SHORT:   cursed_return(unsigned short);
            case GA_TYPELENGTH_SSHORT:  cursed_return(unsigned char);
            case GA_TYPELENGTH_SIZE_T:  cursed_return(size_t);
            default:                    goto endfn;
        }
        case 'f': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_return(float);
            case GA_TYPELENGTH_LONG:    cursed_return(double);
            case GA_TYPELENGTH_LLONG:   cursed_return(long double);
            default:                    goto endfn;
        }
        case 's': switch (self->info->length)
        {
            case GA_TYPELENGTH_NONE:    cursed_return(char *);
            case GA_TYPELENGTH_LONG:    cursed_return(wchar_t *);
            default:                    goto endfn;
        }
        case 'p': cursed_return(void *);
    }
endfn:
    return nullptr;
    #undef cursed_return
}
