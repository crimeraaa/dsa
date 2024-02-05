#include "logger.h"
#include "typeinfo.h"

/**
 * 
 * BEGIN:   MACRO HELL (FUNDAMENTAL TYPEINFO FUNCTIONS)
 * 
 * BRIEF:   It's generally reasonable to want to zero out data. For fundamental
 *          fundamental types, we do just that!
 * 
 * NOTE:    This should also work for structs as well. `(T){0}` works for both
 *          fundamental types and for struct types (as of C11).
 */

#define generate_data(K, T) \
static void *init_##K(void *dst) \
{ \
    *(T*)dst = (T){0}; \
    return dst; \
} \
static void *copy_##K(void *dst, const void *src) \
{ \
    *(T*)dst = *(T*)src;\
    return dst; \
} \
static void *move_##K(void *dst, void *src) \
{ \
    *(T*)dst = *(T*)src; \
    *(T*)src = (T){0}; \
    return dst; \
} \
static void deinit_##K(void *dst) \
{ \
    *(T*)dst = (T){0}; \
} \
const ti_typefns fnlist_##K = { \
    .init = init_##K, \
    .copy = copy_##K, \
    .move = move_##K, \
    .deinit = deinit_##K, \
};
/**
 * END:     MACRO HELL (FUNDAMENTAL TYPE FUNCTIONS)
 */

/* Empty function-like macro argument IS valid! */
#define generate_intdata(callback) \
    callback(, int) \
    callback(l, long) \
    callback(ll, long long) \
    callback(h, short) \
    callback(hh, char)

#define generate_chardata(callback) \
    callback(, char) \
    callback(l, wchar_t)

#define signedfns(K, T)     generate_data(K##i, signed T)
#define unsignedfns(K, T)   generate_data(K##u, unsigned T)
#define charfns(K, T)       generate_data(K##c, T)
#define stringfns(K, T)     generate_data(K##s, T*)

generate_intdata(signedfns)
generate_intdata(unsignedfns)
generate_chardata(charfns)
generate_chardata(stringfns)
generate_data(zu, size_t)
generate_data(p, void *)
    
/** 
 * In order to use `tspec` as an argument for both `fnlist` and `spec`.
 * For `fnlist`, we use it as is via the `##` argument/token expansion.
 * For `spec`, we convert it to a `const char*` and get the first character.
 * 
 * There is no way to conver a macro argument to a character literal:
 * https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Stringification.html
 */
#define generate_ti(ttype, tprefix, tspec, tlength, tsign) { \
    .size = sizeof(ttype), \
    .fnlist = &fnlist_##tprefix##tspec, \
    .length = tlength, \
    .spec = #tspec[0], \
    .is_signed = tsign, \
    .is_fundamental = true, \
}
    
#define generate_LUT_ti(tlength, ttype, tprefix, tspec, tsign) \
    [tlength] = generate_ti(ttype, tprefix, tspec, tlength, tsign),

#define signed_ti(K, T, prefix)    generate_LUT_ti(K, signed T, prefix, i, true)
#define unsigned_ti(K, T, prefix)  generate_LUT_ti(K, unsigned T, prefix, u, false)
    
/** 
 * Not just for int, works for all integral types. 
 * Note that #1 specifically needs the trailing comma to indicate an empty arg.
 */
#define generate_int_ti(callback) \
    callback(TI_LENGTH_NONE,   int,) \
    callback(TI_LENGTH_LONG,   long, l) \
    callback(TI_LENGTH_LLONG,  long long, ll) \
    callback(TI_LENGTH_SHORT,  short, h) \
    callback(TI_LENGTH_SSHORT, char, hh)

#define CHAR_IS_SIGNED  (CHAR_MIN == SCHAR_MIN)
#define WCHAR_IS_SIGNED (WCHAR_MIN != 0)
    
#define char_ti(K, T, prefix, tsign)    generate_LUT_ti(K, T, prefix, c, tsign)
#define string_ti(K, T, prefix, tsign)  generate_LUT_ti(K, T*, prefix, s, tsign)
    
#define generate_char_ti(callback) \
    callback(TI_LENGTH_NONE, char,    ,  CHAR_IS_SIGNED) \
    callback(TI_LENGTH_LONG, wchar_t, l, WCHAR_IS_SIGNED) 

/**
 * END:     MACRO HELL (GENERATING FUNDAMENTAL TYPEINFO)
 */
    
const ti_typelookup ti_fundtypes = {
    .i = {
        generate_int_ti(signed_ti)
    },
    .u = {
        generate_int_ti(unsigned_ti)
        generate_LUT_ti(TI_LENGTH_SIZE_T, size_t, z, u, false)
    },
    .c = {
        generate_char_ti(char_ti)
    },
    .s = {
        generate_char_ti(string_ti)
    },
    .p = generate_ti(void*, , p, TI_LENGTH_NONE, false),
};

const ti_typeinfo *ti_query(char spec, ti_typelength len)
{
    if (len >= TI_LENGTH_COUNT) {
        goto badquery; // Using `goto` so we can also print error message.
    }
    // Order of struct member w/ address-of operator is funky, need parentheses.
    switch (spec)
    {
        case 'i': return &(ti_fundtypes.i[len]);
        case 'u': return &(ti_fundtypes.u[len]);
        case 'c': return &(ti_fundtypes.c[len]);
        case 's': return &(ti_fundtypes.s[len]);
        case 'p': return &(ti_fundtypes.p);
        default:  break;
    }
badquery:
    lg_logprintf("ti_query", "Invalid spec %c and/or len %i", spec, (int)len);
    return nullptr;
}

