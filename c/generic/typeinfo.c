#include "typeinfo.h"

/**
 * BEGIN:   MACRO HELL
  */

#define generate_fund(LUT_Key, C_Type, C_Spec, Is_Signed) \
    [LUT_Key] = {  \
        .size = sizeof(C_Type), \
        .length = LUT_Key, \
        .spec = C_Spec, \
        .sign = Is_Signed, \
        .fund = true, \
    }

#define generate_invalid(K, T) generate_fund(K, T, '\0', false)
#define generate_i(K, T)       generate_fund(K, signed T, 'i', true)
#define generate_u(K, T)       generate_fund(K, unsigned T, 'u', false)

/* Not just for int, works for all integral types. */
#define generate_integral(macrofn) \
    macrofn(TI_LENGTH_NONE, int), \
    macrofn(TI_LENGTH_LONG, long), \
    macrofn(TI_LENGTH_LLONG, long long), \
    macrofn(TI_LENGTH_SHORT, short), \
    macrofn(TI_LENGTH_SSHORT, char),

/**
 * END:     MACRO HELL
 */

const ti_lookup ti_fundtypes = {
    .i = {
        generate_integral(generate_i)
        generate_invalid(TI_LENGTH_SIZE_T, size_t), // No signed size_t
    },
    .u = {
        generate_integral(generate_u)
        generate_fund(TI_LENGTH_SIZE_T, size_t, 'u', false),
    },
    // Actually, most string operations don't care about signedness anyway.
    .c = {
        generate_fund(TI_LENGTH_NONE, char, 'c', (CHAR_MIN == SCHAR_MIN)),
        generate_fund(TI_LENGTH_LONG, wchar_t, 'c', (WCHAR_MIN != 0)),
    },
    .s = {
        generate_fund(TI_LENGTH_NONE, char *, 's', (CHAR_MIN == SCHAR_MIN)),
        generate_fund(TI_LENGTH_LONG, wchar_t *, 's', (WCHAR_MIN != 0)),
    },
    .p = {
        .size = sizeof(void *),
        .length = TI_LENGTH_NONE,
        .spec = 'p',
        .sign = false,
        .fund = true,
    },
};

const ti_typeinfo *ti_query(char spec, ti_typelength len)
{
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
    return nullptr;
}

