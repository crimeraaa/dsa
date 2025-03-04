#include "types.h"

#include <stdint.h>
#include <setjmp.h>

#define lit     string_literal

static const String
TYPE_BASIC_STRINGS[TYPE_BASIC_COUNT] = {
    [TYPE_BASIC_NONE]       = {NULL, 0},
    [TYPE_BASIC_CHAR]       = lit("char"),
    [TYPE_BASIC_SHORT]      = lit("short"),
    [TYPE_BASIC_INT]        = lit("int"),
    [TYPE_BASIC_LONG]       = lit("long"),
    [TYPE_BASIC_LONG_LONG]  = lit("long long"),

    [TYPE_BASIC_FLOAT]      = lit("float"),
    [TYPE_BASIC_DOUBLE]     = lit("double"),
    [TYPE_BASIC_LONG_DOUBLE]= lit("long double"),

    [TYPE_BASIC_VOID]       = lit("void"),
    [TYPE_BASIC_POINTER]    = lit(" *"),

    [TYPE_BASIC_STRUCT]     = lit("struct"),
    [TYPE_BASIC_ENUM]       = lit("enum"),
    [TYPE_BASIC_UNION]      = lit("union"),
};

static const String
TYPE_MOD_STRINGS[TYPE_MOD_COUNT] = {
    [TYPE_MOD_NONE]     = {NULL, 0},
    [TYPE_MOD_SIGNED]   = lit("signed"),
    [TYPE_MOD_UNSIGNED] = lit("unsigned"),
    [TYPE_MOD_COMPLEX]  = lit("complex"),
};

static const String
TYPE_QUAL_STRINGS[TYPE_QUAL_COUNT] = {
    [TYPE_QUAL_CONST]    = lit("const"),
    [TYPE_QUAL_VOLATILE] = lit("volatile"),
    [TYPE_QUAL_RESTRICT] = lit("restrict"),
};

typedef enum {
    TYPE_PARSE_NONE,
    TYPE_PARSE_UNKNOWN, // The word we parsed is not a basic type nor a modifier.
    TYPE_PARSE_INVALID, // The resulting type doesn't make sense, e.g. `long float`, `short char`.
    TYPE_PARSE_COUNT,
} Type_Parse_Error;

typedef struct {
    jmp_buf          caller;
    Type_Parse_Error error;
} Error_Handler;

// `Type_Info` is misleading in my opinion because this is very restrictive.
typedef struct Type_Parse Type_Parse;
struct Type_Parse {
    Type_Parse      *pointee; // Stack-allocated linked list via recursion.
    Error_Handler   *handler;
    Type_Basic       basic;
    Type_Modifier    modifier;
    uint8_t          qualifier;
};

static bool
is_integer(const Type_Parse *info)
{
    return TYPE_BASIC_CHAR <= info->basic && info->basic <= TYPE_BASIC_LONG_LONG;
}

static bool
is_floating(const Type_Parse *info)
{
    return TYPE_BASIC_FLOAT <= info->basic && info->basic <= TYPE_BASIC_LONG_DOUBLE;
}

static bool
is_pointer(const Type_Parse *info)
{
    return info->basic == TYPE_BASIC_POINTER;
}

static bool
has_qualifier(const Type_Parse *info, Type_Qualifier qualifier)
{
    return info->qualifier & BIT(qualifier);
}

static void
set_error(Type_Parse *info, Type_Parse_Error error)
{
    info->handler->error = error;
    if (error == TYPE_PARSE_INVALID)
        longjmp(info->handler->caller, 1);

}

static void
set_basic(Type_Parse *info, String word, Type_Basic expected)
{
    // No error occured because `word` isn't a basic type. Nothing changes.
    // `word` could be an identifier such as an alias or a struct/enum/union.
    if (!string_eq(word, TYPE_BASIC_STRINGS[expected])) {
        set_error(info, TYPE_PARSE_UNKNOWN);
        return;
    }

    // Was previously set?
    if (info->basic != TYPE_BASIC_NONE) switch (info->basic) {
    case TYPE_BASIC_SHORT:
        // Allow `short int`. Do not update `basic->info` because we are
        // already of the correct type.
        if (expected == TYPE_BASIC_INT)
            goto success;
        goto invalid_combination;

    case TYPE_BASIC_INT:
        /**
         * @brief
         *      Allow `int short` and `int long`.
         *
         * @note
         *      In `int long long`, we would have first parsed `int long` so
         *      `basic->type` is `TYPE_BASIC_LONG`, thus we would never reach
         *       here.
         */
        if (expected == TYPE_BASIC_SHORT || expected == TYPE_BASIC_LONG) {
            info->basic = expected;
            goto success;
        }
        goto invalid_combination;

    case TYPE_BASIC_LONG:
        // Allow `long int`. Same idea as `short int`.
        if (expected == TYPE_BASIC_INT) {
            goto success;
        }
        // Allow `long long`.
        else if (expected == TYPE_BASIC_LONG) {
            info->basic = TYPE_BASIC_LONG_LONG;
            goto success;
        }
        // Allow `long double`.
        else if (expected == TYPE_BASIC_DOUBLE) {
            info->basic = TYPE_BASIC_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    // Allow `long long int`.
    case TYPE_BASIC_LONG_LONG:
        if (expected == TYPE_BASIC_INT)
            goto success;
        goto invalid_combination;

    case TYPE_BASIC_DOUBLE:
        if (expected == TYPE_BASIC_LONG) {
            info->basic = TYPE_BASIC_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    default: invalid_combination:
        printfln("ERROR: Invalid type combination " STRING_QFMTSPEC " and \'%s\'",
            STRING_FMTARG(word),
            TYPE_BASIC_STRINGS[expected].data);
        set_error(info, TYPE_PARSE_INVALID);
        return;
    }
    info->basic = expected;
success:
    printfln("Type '%s'", TYPE_BASIC_STRINGS[info->basic].data);
}

static void
set_modifier(Type_Parse *info, String word, Type_Modifier expected)
{
    if (!string_eq(word, TYPE_MOD_STRINGS[expected])) {
        set_error(info, TYPE_PARSE_UNKNOWN);
        return;
    }

    // You cannot repeat or combine modifiers, e.g. `signed signed int` or `signed complex float`.
    if (info->modifier != TYPE_MOD_NONE) {
        printfln("ERROR: Already have modifier " STRING_QFMTSPEC " but got '%s'",
            STRING_FMTARG(word),
            TYPE_MOD_STRINGS[expected].data);
        set_error(info, TYPE_PARSE_INVALID);
    }

    if (info->basic != TYPE_BASIC_NONE) {
        switch (expected) {
        case TYPE_MOD_SIGNED: // fallthrough
        case TYPE_MOD_UNSIGNED:
            if (!is_integer(info))
                goto invalid_combination;
            break;
        case TYPE_MOD_COMPLEX:
            if (!is_floating(info)) invalid_combination: {
                printfln("ERROR: Invalid type and modifier '%s' and '%s'",
                    TYPE_BASIC_STRINGS[info->basic].data,
                    TYPE_MOD_STRINGS[expected].data);
                set_error(info, TYPE_PARSE_INVALID);
                return;
            }
            break;
        default:
            break;
        }
    }

    info->modifier = expected;
    printfln("Modifier: '%s'", TYPE_MOD_STRINGS[expected].data);
    set_error(info, TYPE_PARSE_NONE);
}

static void
set_qualifier(Type_Parse *info, String word, Type_Qualifier expected)
{
    if (!string_eq(word, TYPE_QUAL_STRINGS[expected])) {
        set_error(info, TYPE_PARSE_UNKNOWN);
        return;
    }

    // This qualifier was previously set?
    // `const const int` is valid in C99 and above, but I dislike it.
    // https://stackoverflow.com/questions/5781222/duplicate-const-qualifier-allowed-in-c-but-not-in-c
    if (has_qualifier(info, expected)) {
        printfln("ERROR: Duplicate qualifier '%s'", TYPE_QUAL_STRINGS[expected].data);
        set_error(info, TYPE_PARSE_INVALID);
        return;
    }
    printfln("Qualifier: '%s'", TYPE_QUAL_STRINGS[expected].data);
    info->qualifier |= BIT(expected);
    set_error(info, TYPE_PARSE_NONE);
}

// Ensure modifiers and qualifiers are valid for their basic types.
static void
check_type(Type_Parse *info)
{
    switch (info->modifier) {
    case TYPE_MOD_SIGNED:
    case TYPE_MOD_UNSIGNED:
        if (!is_integer(info))
            goto invalid_combination;
        break;
    case TYPE_MOD_COMPLEX:
        if (!is_floating(info)) invalid_combination: {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_MOD_STRINGS[info->modifier].data,
                TYPE_BASIC_STRINGS[info->basic].data);
            set_error(info, TYPE_PARSE_INVALID);
            return;
        }
        break;
    default:
        break;
    }

    if (has_qualifier(info, TYPE_QUAL_RESTRICT)) {
        if (!is_pointer(info)) {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_QUAL_STRINGS[TYPE_QUAL_RESTRICT].data,
                TYPE_BASIC_STRINGS[info->basic].data);
            set_error(info, TYPE_PARSE_INVALID);
            return;
        }
    }
    set_error(info, TYPE_PARSE_NONE);
    return;
}

static void
finalize_type(Type_Parse *info)
{
    // Map default types for lone modifiers.
    if (info->basic == TYPE_BASIC_NONE) {
        switch (info->modifier) {
        // `signed` maps to `signed int` which in turn maps to `int`.
        case TYPE_MOD_SIGNED:
        // `unsigned` maps to `unsigned int`.
        case TYPE_MOD_UNSIGNED:
            info->basic = TYPE_BASIC_INT;
            break;

        // `complex` maps to `complex double`.
        case TYPE_MOD_COMPLEX:
            info->basic = TYPE_BASIC_DOUBLE;
            break;

        default:
            println("ERROR: No basic type nor modifier were received.");
            set_error(info, TYPE_PARSE_INVALID);
            return;
        }
    }

    // Ensure all the integer types (sans `char`) are signed when not specified.
    switch (info->basic) {
    case TYPE_BASIC_SHORT:
    case TYPE_BASIC_INT:
    case TYPE_BASIC_LONG:
    case TYPE_BASIC_LONG_LONG:
        if (info->modifier == TYPE_MOD_NONE)
            info->modifier = TYPE_MOD_SIGNED;
        break;

    default:
        break;
    }
    check_type(info);
}

static void
print_qualifier(const Type_Parse *info, Type_Qualifier qualifier)
{
    // Don't use `info.qualifier` as an index as it contains bit flags!
    if (has_qualifier(info, qualifier))
        printf(" %s", TYPE_QUAL_STRINGS[qualifier].data);
}

static void
print_type(const Type_Parse *info)
{
    // Print from the innermost (i.e; most pointed-to) going outermost.
    if (info->pointee)
        print_type(info->pointee);

    // Innermost type (the very first pointee) is where we start printing.
    switch (info->modifier) {
    case TYPE_MOD_NONE: print_basic:
        printf("%s", TYPE_BASIC_STRINGS[info->basic].data);
        break;
    case TYPE_MOD_SIGNED:
        // all signed types (except for `char`) map to their base types.
        // e.g. `signed int` maps to `int`.
        if (info->basic != TYPE_BASIC_CHAR)
            goto print_basic;
        // Fallthrough
    case TYPE_MOD_UNSIGNED:
    case TYPE_MOD_COMPLEX:
        printf("%s %s", TYPE_MOD_STRINGS[info->modifier].data, TYPE_BASIC_STRINGS[info->basic].data);
        break;
    default:
        break;
    }

    print_qualifier(info, TYPE_QUAL_CONST);
    print_qualifier(info, TYPE_QUAL_VOLATILE);
    print_qualifier(info, TYPE_QUAL_RESTRICT);
}

static void
parse_with_state(Type_Parse *info, String *state, int recurse)
{
    // Help visualize each step of the parsing process.
    int step = 1;
    for (String word; string_split_whitespace_iterator(&word, state);) {
        for (int tabs = 0; tabs < recurse; ++tabs) {
            fputc('\t', stdout);
        }
        printf("[%i]: " STRING_QFMTSPEC " => ", step++, STRING_FMTARG(word));
        switch (word.data[0]) {
        case 'c':
            set_basic(info, word, TYPE_BASIC_CHAR);
            set_modifier(info, word, TYPE_MOD_COMPLEX);
            set_qualifier(info, word, TYPE_QUAL_CONST);
            break;
        case 'd':
            set_basic(info, word, TYPE_BASIC_DOUBLE);
            break;
        case 'f':
            set_basic(info, word, TYPE_BASIC_FLOAT);
            break;
        case 'i':
            set_basic(info, word, TYPE_BASIC_INT);
            break;
        case 'l':
            set_basic(info, word, TYPE_BASIC_LONG);
            break;
        case 'r':
            set_qualifier(info, word, TYPE_QUAL_RESTRICT);
            break;
        case 's':
            set_basic(info, word, TYPE_BASIC_SHORT);
            set_modifier(info, word, TYPE_MOD_SIGNED);
            break;
        case 'u':
            set_modifier(info, word, TYPE_MOD_UNSIGNED);
            break;
        case 'v':
            set_basic(info, word, TYPE_BASIC_VOID);
            set_qualifier(info, word, TYPE_QUAL_VOLATILE);
            break;
        case '*': {
            // e.g. in `int *const`, `state` currently points to EOF, make it
            // point to `const`, skipping the `*`.
            state->data  = word.data + 1;
            state->len  += word.len - 1;
            Type_Parse pointer = {
                .pointee   = info,
                .handler   = info->handler,
                .basic     = TYPE_BASIC_POINTER,
                .modifier  = TYPE_MOD_NONE,
                .qualifier = 0,
            };
            finalize_type(info);
            printf("Pointer to '");
            print_type(info);
            printf("'\n");
            // Return because we only want to print the deepest recursive call.
            parse_with_state(&pointer, state, recurse + 1);
            return;
        }
        default:
            set_error(info, TYPE_PARSE_UNKNOWN);
            break;
        }

        if (info->handler->error == TYPE_PARSE_UNKNOWN)
            println("Identifier(?)");
    }
    finalize_type(info);
    print_type(info);
    printf("\n");
}

bool
type_parse_string(String text)
{
    Error_Handler handler;
    handler.error = TYPE_PARSE_NONE;

    Type_Parse info  = {
        .pointee   = NULL,
        .handler   = &handler,
        .basic     = TYPE_BASIC_NONE,
        .modifier  = TYPE_MOD_NONE,
        .qualifier = 0,
    };

    if (setjmp(handler.caller) == 0) {
        String state = text;
        parse_with_state(&info, &state, 1);
        return true;
    } else {
        return false;
    }
}
