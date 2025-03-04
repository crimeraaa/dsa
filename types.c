#include "types.h"

#include <stdint.h>
#include <setjmp.h>
#include <string.h>

#define lit     string_literal

static const String
TYPE_BASE_STRINGS[TYPE_BASE_COUNT] = {
    [TYPE_BASE_NONE]       = {NULL, 0},
    [TYPE_BASE_CHAR]       = lit("char"),
    [TYPE_BASE_SHORT]      = lit("short"),
    [TYPE_BASE_INT]        = lit("int"),
    [TYPE_BASE_LONG]       = lit("long"),
    [TYPE_BASE_LONG_LONG]  = lit("long long"),

    [TYPE_BASE_FLOAT]      = lit("float"),
    [TYPE_BASE_DOUBLE]     = lit("double"),
    [TYPE_BASE_LONG_DOUBLE]= lit("long double"),

    [TYPE_BASE_VOID]       = lit("void"),
    [TYPE_BASE_POINTER]    = {NULL, 0},

    [TYPE_BASE_STRUCT]     = lit("struct"),
    [TYPE_BASE_ENUM]       = lit("enum"),
    [TYPE_BASE_UNION]      = lit("union"),
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
    Type_Parse         *pointee; // Stack-allocated linked list via recursion.
    Type_Table         *table;
    Error_Handler      *handler;
    Type_Base          basic;
    Type_Modifier       modifier;
    Type_Qualifier_Set  qualifiers;
};

static bool
_parse_is_integer(const Type_Parse *parse)
{
    return TYPE_BASE_CHAR <= parse->basic && parse->basic <= TYPE_BASE_LONG_LONG;
}

static bool
_parse_is_floating(const Type_Parse *parse)
{
    return TYPE_BASE_FLOAT <= parse->basic && parse->basic <= TYPE_BASE_LONG_DOUBLE;
}

static bool
_parse_is_pointer(const Type_Parse *parse)
{
    return parse->basic == TYPE_BASE_POINTER;
}

static bool
_parse_has_qualifier(const Type_Parse *parse, Type_Qualifier qualifier)
{
    return parse->qualifiers & BIT(qualifier);
}

static void
_parse_set_error(Type_Parse *parse, Type_Parse_Error error)
{
    parse->handler->error = error;
    if (error == TYPE_PARSE_INVALID)
        longjmp(parse->handler->caller, 1);

}

static void
_parse_set_basic(Type_Parse *parse, String word, Type_Base expected)
{
    // No error occured because `word` isn't a basic type. Nothing changes.
    // `word` could be an identifier such as an alias or a struct/enum/union.
    if (!string_eq(word, TYPE_BASE_STRINGS[expected])) {
        _parse_set_error(parse, TYPE_PARSE_UNKNOWN);
        return;
    }

    // Was previously set?
    if (parse->basic != TYPE_BASE_NONE) switch (parse->basic) {
    case TYPE_BASE_SHORT:
        // Allow `short int`. Do not update `basic->info` because we are
        // already of the correct type.
        if (expected == TYPE_BASE_INT)
            goto success;
        goto invalid_combination;

    case TYPE_BASE_INT:
        /**
         * @brief
         *      Allow `int short` and `int long`.
         *
         * @note
         *      In `int long long`, we would have first parsed `int long` so
         *      `basic->type` is `TYPE_BASE_LONG`, thus we would never reach
         *       here.
         */
        if (expected == TYPE_BASE_SHORT || expected == TYPE_BASE_LONG) {
            parse->basic = expected;
            goto success;
        }
        goto invalid_combination;

    case TYPE_BASE_LONG:
        // Allow `long int`. Same idea as `short int`.
        if (expected == TYPE_BASE_INT) {
            goto success;
        }
        // Allow `long long`.
        else if (expected == TYPE_BASE_LONG) {
            parse->basic = TYPE_BASE_LONG_LONG;
            goto success;
        }
        // Allow `long double`.
        else if (expected == TYPE_BASE_DOUBLE) {
            parse->basic = TYPE_BASE_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    // Allow `long long int`.
    case TYPE_BASE_LONG_LONG:
        if (expected == TYPE_BASE_INT)
            goto success;
        goto invalid_combination;

    case TYPE_BASE_DOUBLE:
        if (expected == TYPE_BASE_LONG) {
            parse->basic = TYPE_BASE_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    default: invalid_combination:
        printfln("ERROR: Invalid type combination " STRING_QFMTSPEC " and \'%s\'",
            STRING_FMTARG(word),
            TYPE_BASE_STRINGS[expected].data);
        _parse_set_error(parse, TYPE_PARSE_INVALID);
        return;
    }
    parse->basic = expected;
success:
    printfln("Type '%s'", TYPE_BASE_STRINGS[parse->basic].data);
}

static void
_parse_set_modifier(Type_Parse *parse, String word, Type_Modifier expected)
{
    if (!string_eq(word, TYPE_MOD_STRINGS[expected])) {
        _parse_set_error(parse, TYPE_PARSE_UNKNOWN);
        return;
    }

    // You cannot repeat or combine modifiers, e.g. `signed signed int` or `signed complex float`.
    if (parse->modifier != TYPE_MOD_NONE) {
        printfln("ERROR: Already have modifier " STRING_QFMTSPEC " but got '%s'",
            STRING_FMTARG(word),
            TYPE_MOD_STRINGS[expected].data);
        _parse_set_error(parse, TYPE_PARSE_INVALID);
    }

    if (parse->basic != TYPE_BASE_NONE) {
        switch (expected) {
        case TYPE_MOD_SIGNED: // fallthrough
        case TYPE_MOD_UNSIGNED:
            if (!_parse_is_integer(parse))
                goto invalid_combination;
            break;
        case TYPE_MOD_COMPLEX:
            if (!_parse_is_floating(parse)) invalid_combination: {
                printfln("ERROR: Invalid type and modifier '%s' and '%s'",
                    TYPE_BASE_STRINGS[parse->basic].data,
                    TYPE_MOD_STRINGS[expected].data);
                _parse_set_error(parse, TYPE_PARSE_INVALID);
                return;
            }
            break;
        default:
            break;
        }
    }

    parse->modifier = expected;
    printfln("Modifier: '%s'", TYPE_MOD_STRINGS[expected].data);
    _parse_set_error(parse, TYPE_PARSE_NONE);
}

static void
_parse_set_qualifier(Type_Parse *parse, String word, Type_Qualifier expected)
{
    if (!string_eq(word, TYPE_QUAL_STRINGS[expected])) {
        _parse_set_error(parse, TYPE_PARSE_UNKNOWN);
        return;
    }

    // This qualifier was previously set?
    // `const const int` is valid in C99 and above, but I dislike it.
    // https://stackoverflow.com/questions/5781222/duplicate-const-qualifier-allowed-in-c-but-not-in-c
    if (_parse_has_qualifier(parse, expected)) {
        printfln("ERROR: Duplicate qualifier '%s'", TYPE_QUAL_STRINGS[expected].data);
        _parse_set_error(parse, TYPE_PARSE_INVALID);
        return;
    }
    printfln("Qualifier: '%s'", TYPE_QUAL_STRINGS[expected].data);
    parse->qualifiers |= BIT(expected);
    _parse_set_error(parse, TYPE_PARSE_NONE);
}

// Ensure modifiers and qualifiers are valid for their basic types.
static void
_parse_check(Type_Parse *parse)
{
    switch (parse->modifier) {
    case TYPE_MOD_SIGNED:
    case TYPE_MOD_UNSIGNED:
        if (!_parse_is_integer(parse))
            goto invalid_combination;
        break;
    case TYPE_MOD_COMPLEX:
        if (!_parse_is_floating(parse)) invalid_combination: {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_MOD_STRINGS[parse->modifier].data,
                TYPE_BASE_STRINGS[parse->basic].data);
            _parse_set_error(parse, TYPE_PARSE_INVALID);
            return;
        }
        break;
    default:
        break;
    }

    if (_parse_has_qualifier(parse, TYPE_QUAL_RESTRICT)) {
        if (!_parse_is_pointer(parse)) {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_QUAL_STRINGS[TYPE_QUAL_RESTRICT].data,
                TYPE_BASE_STRINGS[parse->basic].data);
            _parse_set_error(parse, TYPE_PARSE_INVALID);
            return;
        }
    }
    _parse_set_error(parse, TYPE_PARSE_NONE);
    return;
}

static void
_parse_finalize(Type_Parse *parse)
{
    // Map default types for lone modifiers.
    if (parse->basic == TYPE_BASE_NONE) {
        switch (parse->modifier) {
        // `signed` maps to `signed int` which in turn maps to `int`.
        case TYPE_MOD_SIGNED:
        // `unsigned` maps to `unsigned int`.
        case TYPE_MOD_UNSIGNED:
            parse->basic = TYPE_BASE_INT;
            break;

        // `complex` maps to `complex double`.
        case TYPE_MOD_COMPLEX:
            parse->basic = TYPE_BASE_DOUBLE;
            break;

        default:
            println("ERROR: No basic type nor modifier were received.");
            _parse_set_error(parse, TYPE_PARSE_INVALID);
            return;
        }
    }

    // Ensure all the integer types (sans `char`) are signed when not specified.
    switch (parse->basic) {
    case TYPE_BASE_SHORT:
    case TYPE_BASE_INT:
    case TYPE_BASE_LONG:
    case TYPE_BASE_LONG_LONG:
        if (parse->modifier == TYPE_MOD_NONE)
            parse->modifier = TYPE_MOD_SIGNED;
        break;

    default:
        break;
    }
    _parse_check(parse);
}

static void
_parse_write_qualifier(const Type_Parse *parse, String_Builder *builder, Type_Qualifier qualifier)
{
    if (parse->qualifiers & BIT(qualifier)) {
        string_append_string(builder, TYPE_QUAL_STRINGS[qualifier]);
        string_append_char(builder, ' ');
    }
}

static void
_parse_try_write_qualifiers(const Type_Parse *parse, String_Builder *builder)
{
    _parse_write_qualifier(parse, builder, TYPE_QUAL_CONST);
    _parse_write_qualifier(parse, builder, TYPE_QUAL_VOLATILE);
    _parse_write_qualifier(parse, builder, TYPE_QUAL_RESTRICT);
}

static void
_parse_write(const Type_Parse *parse)
{
    // We will only do this for up to the first pointer. E.g. given `int *const`,
    // we will first build the string `int *`.
    char buf[256];
    String_Builder builder = string_builder_make_fixed(buf, sizeof buf);

    if (parse->pointee)
        _parse_write(parse->pointee);

    if (!_parse_is_pointer(parse))
        _parse_try_write_qualifiers(parse, &builder);

    switch (parse->modifier) {
    case TYPE_MOD_NONE: write_type_only:
        string_append_string(&builder, TYPE_BASE_STRINGS[parse->basic]);
        break;
    case TYPE_MOD_SIGNED:
        if (parse->basic != TYPE_BASE_CHAR)
            goto write_type_only;
        // Fallthrough
    case TYPE_MOD_UNSIGNED:
    case TYPE_MOD_COMPLEX:
        string_append_string(&builder, TYPE_MOD_STRINGS[parse->modifier]);
        string_append_char(&builder, ' ');
        string_append_string(&builder, TYPE_BASE_STRINGS[parse->basic]);
        break;
    default:
        break;
    }

    if (_parse_is_pointer(parse)) {
        string_append_cstring(&builder, " *");
        _parse_try_write_qualifiers(parse, &builder);
    }

    printf("%s", string_to_cstring(&builder));
}

static void
_parse_with_state(Type_Parse *parse, String *state, int recurse)
{
    // Help visualize each step of the parsing process.
    int step = 1;
    for (String word; string_split_whitespace_iterator(&word, state);) {
        printfln("\tword = " STRING_QFMTSPEC ", state = " STRING_QFMTSPEC,
                STRING_FMTARG(word),
                STRING_FMTARG(*state));
        for (int tabs = 0; tabs < recurse; ++tabs) {
            fputc('\t', stdout);
        }
        printf("[%i]: " STRING_QFMTSPEC " => ", step++, STRING_FMTARG(word));
        switch (word.data[0]) {
        case 'c':
            _parse_set_basic(parse, word, TYPE_BASE_CHAR);
            _parse_set_modifier(parse, word, TYPE_MOD_COMPLEX);
            _parse_set_qualifier(parse, word, TYPE_QUAL_CONST);
            break;
        case 'd':
            _parse_set_basic(parse, word, TYPE_BASE_DOUBLE);
            break;
        case 'f':
            _parse_set_basic(parse, word, TYPE_BASE_FLOAT);
            break;
        case 'i':
            _parse_set_basic(parse, word, TYPE_BASE_INT);
            break;
        case 'l':
            _parse_set_basic(parse, word, TYPE_BASE_LONG);
            break;
        case 'r':
            _parse_set_qualifier(parse, word, TYPE_QUAL_RESTRICT);
            break;
        case 's':
            _parse_set_basic(parse, word, TYPE_BASE_SHORT);
            _parse_set_modifier(parse, word, TYPE_MOD_SIGNED);
            break;
        case 'u':
            _parse_set_modifier(parse, word, TYPE_MOD_UNSIGNED);
            break;
        case 'v':
            _parse_set_basic(parse, word, TYPE_BASE_VOID);
            _parse_set_qualifier(parse, word, TYPE_QUAL_VOLATILE);
            break;
        case '*': {
            // e.g. in `int *const`, `state` currently points to EOF, make it
            // point to `const`, skipping the `*`.
            state->data  = word.data + 1;
            state->len  += word.len - 1;
            Type_Parse pointer = {
                .pointee    = parse,
                .handler    = parse->handler,
                .table      = parse->table,
                .basic      = TYPE_BASE_POINTER,
                .modifier   = TYPE_MOD_NONE,
                .qualifiers = 0,
            };
            _parse_finalize(parse);
            printf("Pointer to '");
            _parse_write(parse);
            printf("'\n");
            // Return because we only want to print the deepest recursive call.
            _parse_with_state(&pointer, state, recurse + 1);
            return;
        }
        default:
            _parse_set_error(parse, TYPE_PARSE_UNKNOWN);
            break;
        }

        if (parse->handler->error == TYPE_PARSE_UNKNOWN)
            println("Identifier(?)");
    }
    _parse_finalize(parse);
    _parse_write(parse);
    printf("\n");
}

bool
type_parse_string(Type_Table *table, const char *text, size_t len)
{
    Error_Handler handler;
    handler.error = TYPE_PARSE_NONE;

    Type_Parse parse = {
        .pointee    = NULL,
        .handler    = &handler,
        .table      = table,
        .basic      = TYPE_BASE_NONE,
        .modifier   = TYPE_MOD_NONE,
        .qualifiers = 0,
    };

    if (setjmp(handler.caller) == 0) {
        String state = {text, len};
        _parse_with_state(&parse, &state, 1);
        return true;
    } else {
        return false;
    }
}

static const struct {
    Type_Base    basic;
    const char   *base_name;
} TYPE_INFO_INTEGERS[] = {
    {.basic = TYPE_BASE_CHAR,      .base_name = "char"},
    {.basic = TYPE_BASE_SHORT,     .base_name = "short"},
    {.basic = TYPE_BASE_INT,       .base_name = "int"},
    {.basic = TYPE_BASE_LONG,      .base_name = "long"},
    {.basic = TYPE_BASE_LONG_LONG, .base_name = "long long"},
};

Type_Table
type_table_make(Allocator allocator)
{
    Type_Table table = {
        .intern  = intern_make(allocator),
        .entries = NULL,
        .len     = 0,
        .cap     = 0,
    };

    for (size_t i = 0; i < count_of(TYPE_INFO_INTEGERS); ++i) {
        Type_Info info = {
            .basic = TYPE_INFO_INTEGERS[i].basic,
            .integer = {.modifier = TYPE_MOD_SIGNED, .qualifiers = 0},
        };
        char buf[64];
        String_Builder builder = string_builder_make_fixed(buf, sizeof buf);
        string_append_string(&builder, TYPE_BASE_STRINGS[info.basic]);
        if (info.basic == TYPE_BASE_CHAR) {
            info.integer.modifier = TYPE_MOD_NONE;
        }
        type_add(&table, string_to_cstring(&builder), info);

        // `signed char` is distinct from `char`.
        string_prepend_char(&builder, ' ');
        string_prepend_string(&builder, TYPE_MOD_STRINGS[info.integer.modifier = TYPE_MOD_SIGNED]);
        if (info.basic == TYPE_BASE_CHAR)
            type_add(&table, string_to_cstring(&builder), info);

        // All `unsigned` types are distinct from their base types.
        info.integer.modifier = TYPE_MOD_UNSIGNED;
        string_prepend_cstring(&builder, "un");
        type_add(&table, string_to_cstring(&builder), info);
    }
    type_table_print(&table);
    return table;
}

static void
_print_qualifier(const Type_Info *info, Type_Qualifier qualifier)
{
    if (info->integer.qualifiers & BIT(qualifier)) {
        printf("%s", TYPE_QUAL_STRINGS[qualifier].data);
    }
}

void
type_table_print(const Type_Table *table)
{
    for (size_t i = 0, len = table->len; i < len; ++i) {
        Type_Info *info = table->entries[i].info;
        printf("[%18s] = {modifier = %s, qualifiers = {",
                table->entries[i].base_name->data,
                TYPE_MOD_STRINGS[info->integer.modifier].data);

        _print_qualifier(info, TYPE_QUAL_CONST);
        _print_qualifier(info, TYPE_QUAL_VOLATILE);
        _print_qualifier(info, TYPE_QUAL_RESTRICT);
        println("}}");
    }
}

void
type_table_destroy(Type_Table *table)
{
    Allocator allocator = table->intern.allocator;
    for (size_t i = 0, end = table->len; i < end; ++i) {
        mem_free(table->entries[i].info, allocator);
    }

    mem_delete(table->entries, table->cap, allocator);
    intern_destroy(&table->intern);
    table->entries = NULL;
    table->len     = 0;
    table->cap     = 0;
}

static bool
type_info_eq(const Type_Info *a, const Type_Info *b)
{
    if (a->basic != b->basic)
        return false;

    switch (a->basic) {
    // Integer
    case TYPE_BASE_CHAR:
    case TYPE_BASE_SHORT:
    case TYPE_BASE_INT:
    case TYPE_BASE_LONG:
    case TYPE_BASE_LONG_LONG:
        return a->integer.modifier   == b->integer.modifier
            && a->integer.qualifiers == b->integer.qualifiers;

    // Floating
    case TYPE_BASE_FLOAT:
    case TYPE_BASE_DOUBLE:
    case TYPE_BASE_LONG_DOUBLE:
        return a->floating.modifier   == b->floating.modifier
            && a->floating.qualifiers == b->floating.qualifiers;

    // Misc.
    case TYPE_BASE_VOID:
        break;
    case TYPE_BASE_POINTER:
        return a->pointer.pointee    == b->pointer.pointee
            && a->pointer.qualifiers == b->pointer.qualifiers;

    // User defined
    case TYPE_BASE_STRUCT:
    case TYPE_BASE_ENUM:
    case TYPE_BASE_UNION:
        break;
    default:
        break;
    }
    return false;
}

Allocator_Error
type_add(Type_Table *table, const char *base_name, Type_Info info)
{
    const size_t len = table->len;
    for (size_t i = 0; i < len; ++i) {
        Type_Info *other = table->entries[i].info;
        if (type_info_eq(&info, other))
            return ALLOCATOR_ERROR_NONE;
    }

    // Need to grow the dynamic array?
    const size_t cap = table->cap;
    if (len >= cap) {
        size_t            new_cap     = (cap == 0) ? 8 : cap * 2;
        Type_Table_Entry *new_entries = mem_resize(Type_Table_Entry, table->entries, cap, new_cap, table->intern.allocator);
        if (new_entries == NULL)
            return ALLOCATOR_ERROR_OUT_OF_MEMORY;
        table->entries = new_entries;
        table->cap     = new_cap;
    }

    Type_Info *new_info = mem_new(Type_Info, table->intern.allocator);
    if (new_info == NULL)
        return ALLOCATOR_ERROR_OUT_OF_MEMORY;
    *new_info = info;

    Type_Table_Entry *entry = &table->entries[table->len++];
    entry->info      = new_info;
    entry->base_name = intern_get_interned(&table->intern, string_from_cstring(base_name));
    return ALLOCATOR_ERROR_NONE;
}

const Type_Info *
type_get_by_name(Type_Table *table, const char *name)
{
    const Intern_String *query = intern_get_interned(&table->intern, string_from_cstring(name));
    for (size_t i = 0, len = table->len; i < len; ++i) {
        if (query == table->entries[i].base_name) {
            return table->entries[i].info;
        }
    }
    return NULL;
}

const Type_Info *
type_get_by_info(Type_Table *table, Type_Info info)
{
    for (size_t i = 0, len = table->len; i < len; ++i) {
        const Type_Info *other = table->entries[i].info;
        if (type_info_eq(&info, other)) {
            return other;
        }
    }
    return NULL;
}
