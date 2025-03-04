#include "parser.h"

#include <assert.h>

static bool
_parser_is_integer(const Type_Parser *parser)
{
    return TYPE_BASE_CHAR <= parser->data->basic && parser->data->basic <= TYPE_BASE_LONG_LONG;
}

static bool
_parser_is_floating(const Type_Parser *parser)
{
    return TYPE_BASE_FLOAT <= parser->data->basic && parser->data->basic <= TYPE_BASE_LONG_DOUBLE;
}

static bool
_parser_is_pointer(const Type_Parser *parser)
{
    return parser->data->basic == TYPE_BASE_POINTER;
}

static bool
_parser_has_qualifier(const Type_Parser *parser, Type_Qualifier qualifier)
{
    return parser->data->qualifiers & BIT(qualifier);
}

static void
_parser_throw_error(Type_Parser *parser, Type_Parse_Error error)
{
    parser->handler->error = error;
    longjmp(parser->handler->caller, 1);

}

static void
_parser_set_basic(Type_Parser *parser, Type_Token token, Type_Base expected)
{
    // Was previously set?
    if (parser->data->basic != TYPE_BASE_NONE) switch (parser->data->basic) {
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
            parser->data->basic = expected;
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
            parser->data->basic = TYPE_BASE_LONG_LONG;
            goto success;
        }
        // Allow `long double`.
        else if (expected == TYPE_BASE_DOUBLE) {
            parser->data->basic = TYPE_BASE_LONG_DOUBLE;
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
            parser->data->basic = TYPE_BASE_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    default: invalid_combination:
        printfln("ERROR: Invalid type combination " STRING_QFMTSPEC " and \'%s\'",
            STRING_FMTARG(token.word),
            TYPE_BASE_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
        return;
    }
    parser->data->basic = expected;
success:
    printfln("Type '%s'", TYPE_BASE_STRINGS[parser->data->basic].data);
}

static void
_parser_set_modifier(Type_Parser *parser, Type_Token token, Type_Modifier expected)
{
    // You cannot repeat or combine modifiers, e.g. `signed signed int` or `signed complex float`.
    if (parser->data->modifier != TYPE_MOD_NONE) {
        printfln("ERROR: Already have modifier " STRING_QFMTSPEC " but got '%s'",
            STRING_FMTARG(token.word),
            TYPE_MOD_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
    }

    if (parser->data->basic != TYPE_BASE_NONE) {
        switch (expected) {
        case TYPE_MOD_SIGNED: // fallthrough
        case TYPE_MOD_UNSIGNED:
            if (!_parser_is_integer(parser))
                goto invalid_combination;
            break;
        case TYPE_MOD_COMPLEX:
            if (!_parser_is_floating(parser)) invalid_combination: {
                printfln("ERROR: Invalid type and modifier '%s' and '%s'",
                    TYPE_BASE_STRINGS[parser->data->basic].data,
                    TYPE_MOD_STRINGS[expected].data);
                _parser_throw_error(parser, TYPE_PARSE_INVALID);
                return;
            }
            break;
        default:
            break;
        }
    }

    parser->data->modifier = expected;
    printfln("Modifier: '%s'", TYPE_MOD_STRINGS[expected].data);
}

static void
_parser_set_qualifier(Type_Parser *parser, Type_Qualifier expected)
{
    // This qualifier was previously set?
    // `const const int` is valid in C99 and above, but I dislike it.
    // https://stackoverflow.com/questions/5781222/duplicate-const-qualifier-allowed-in-c-but-not-in-c
    if (_parser_has_qualifier(parser, expected)) {
        printfln("ERROR: Duplicate qualifier '%s'", TYPE_QUAL_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
        return;
    }
    printfln("Qualifier: '%s'", TYPE_QUAL_STRINGS[expected].data);
    parser->data->qualifiers |= BIT(expected);
}

// Ensure modifiers and qualifiers are valid for their basic types.
static void
_parser_check(Type_Parser *parser)
{
    switch (parser->data->modifier) {
    case TYPE_MOD_SIGNED:
    case TYPE_MOD_UNSIGNED:
        if (!_parser_is_integer(parser))
            goto invalid_combination;
        break;
    case TYPE_MOD_COMPLEX:
        if (!_parser_is_floating(parser)) invalid_combination: {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_MOD_STRINGS[parser->data->modifier].data,
                TYPE_BASE_STRINGS[parser->data->basic].data);
            _parser_throw_error(parser, TYPE_PARSE_INVALID);
            return;
        }
        break;
    default:
        break;
    }

    if (_parser_has_qualifier(parser, TYPE_QUAL_RESTRICT)) {
        if (!_parser_is_pointer(parser)) {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_QUAL_STRINGS[TYPE_QUAL_RESTRICT].data,
                TYPE_BASE_STRINGS[parser->data->basic].data);
            _parser_throw_error(parser, TYPE_PARSE_INVALID);
            return;
        }
    }
    return;
}

static void
_parser_finalize(Type_Parser *parser, bool final)
{
    // Map default types for lone modifiers.
    if (parser->data->basic == TYPE_BASE_NONE) {
        switch (parser->data->modifier) {
        // `signed` maps to `signed int` which in turn maps to `int`.
        case TYPE_MOD_SIGNED:
        // `unsigned` maps to `unsigned int`.
        case TYPE_MOD_UNSIGNED:
            parser->data->basic = TYPE_BASE_INT;
            break;

        // `complex` maps to `complex double`.
        case TYPE_MOD_COMPLEX:
            parser->data->basic = TYPE_BASE_DOUBLE;
            break;

        default:
            if (final) {
                println("ERROR: No basic type nor modifier were received.");
                _parser_throw_error(parser, TYPE_PARSE_INVALID);
            }
            return;
        }
    }

    // Ensure all the integer types (sans `char`) are signed when not specified.
    switch (parser->data->basic) {
    case TYPE_BASE_SHORT:
    case TYPE_BASE_INT:
    case TYPE_BASE_LONG:
    case TYPE_BASE_LONG_LONG:
        if (parser->data->modifier == TYPE_MOD_NONE)
            parser->data->modifier = TYPE_MOD_SIGNED;
        break;

    default:
        break;
    }
    _parser_check(parser);
}

static void
_parser_write_qualifier(const Type_Parser *parser, String_Builder *builder, Type_Qualifier qualifier)
{
    if (parser->data->qualifiers & BIT(qualifier)) {
        string_append_string(builder, TYPE_QUAL_STRINGS[qualifier]);
        string_append_char(builder, ' ');
    }
}

static void
_parser_try_write_qualifiers(const Type_Parser *parser, String_Builder *builder)
{
    _parser_write_qualifier(parser, builder, TYPE_QUAL_CONST);
    _parser_write_qualifier(parser, builder, TYPE_QUAL_VOLATILE);
    _parser_write_qualifier(parser, builder, TYPE_QUAL_RESTRICT);
}

static void
_parser_write(Type_Parser *parser)
{
    // We will only do this for up to the first pointer. E.g. given `int *const`,
    // we will first build the string `int *`.
    char buf[256];
    String_Builder builder = string_builder_make_fixed(buf, sizeof buf);

    if (parser->data->pointee != NULL) {
        // hack because we keep so much state
        Type_Parser_Data *prev = parser->data;
        parser->data = parser->data->pointee;
        _parser_write(parser);
        parser->data = prev;
    }

    if (!_parser_is_pointer(parser))
        _parser_try_write_qualifiers(parser, &builder);

    switch (parser->data->modifier) {
    case TYPE_MOD_NONE: write_type_only:
        string_append_string(&builder, TYPE_BASE_STRINGS[parser->data->basic]);
        break;
    case TYPE_MOD_SIGNED:
        if (parser->data->basic != TYPE_BASE_CHAR)
            goto write_type_only;
        // Fallthrough
    case TYPE_MOD_UNSIGNED:
    case TYPE_MOD_COMPLEX:
        string_append_string(&builder, TYPE_MOD_STRINGS[parser->data->modifier]);
        string_append_char(&builder, ' ');
        string_append_string(&builder, TYPE_BASE_STRINGS[parser->data->basic]);
        break;
    default:
        break;
    }

    if (_parser_is_pointer(parser)) {
        string_append_cstring(&builder, " *");
        _parser_try_write_qualifiers(parser, &builder);
    }

    printf("%s", string_to_cstring(&builder));
}

void
type_parser_parse(Type_Parser *parser, String *state, int recurse)
{
    Type_Token token;
    
loop_start:
    // Ugly to use goto like this but it works for our purposes...
    token = type_lexer_scan(&parser->lexer);
    parser->consumed = token;
    switch (token.type) {
    // Integer
    case TYPE_TOKEN_CHAR:   _parser_set_basic(parser, token, TYPE_BASE_CHAR);   break;
    case TYPE_TOKEN_SHORT:  _parser_set_basic(parser, token, TYPE_BASE_SHORT);  break;
    case TYPE_TOKEN_INT:    _parser_set_basic(parser, token, TYPE_BASE_INT);    break;
    case TYPE_TOKEN_LONG:   _parser_set_basic(parser, token, TYPE_BASE_LONG);   break;

    // Float
    case TYPE_TOKEN_FLOAT:  _parser_set_basic(parser, token, TYPE_BASE_FLOAT);  break;
    case TYPE_TOKEN_DOUBLE: _parser_set_basic(parser, token, TYPE_BASE_DOUBLE); break;

    // User-defined
    case TYPE_TOKEN_STRUCT:
    case TYPE_TOKEN_ENUM:
    case TYPE_TOKEN_UNION:
    case TYPE_TOKEN_IDENT:
        printfln("ERROR '%s' not yet supported!", TYPE_TOKEN_STRINGS[token.type].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
        break;

    case TYPE_TOKEN_SIGNED:     _parser_set_modifier(parser, token, TYPE_MOD_SIGNED);   break;
    case TYPE_TOKEN_UNSIGNED:   _parser_set_modifier(parser, token, TYPE_MOD_UNSIGNED); break;
    case TYPE_TOKEN_COMPLEX:    _parser_set_modifier(parser, token, TYPE_MOD_COMPLEX);  break;
        break;

    case TYPE_TOKEN_CONST:      _parser_set_qualifier(parser, TYPE_QUAL_CONST); break;
    case TYPE_TOKEN_VOLATILE:   _parser_set_qualifier(parser, TYPE_QUAL_VOLATILE); break;
    case TYPE_TOKEN_RESTRICT:   _parser_set_qualifier(parser, TYPE_QUAL_RESTRICT); break;
        break;

    case TYPE_TOKEN_VOID:       _parser_set_basic(parser, token, TYPE_BASE_VOID); break;
    case TYPE_TOKEN_ASTERISK: {
        Type_Parser_Data pointer = {
            .pointee    = parser->data,
            .basic      = TYPE_BASE_POINTER,
            .modifier   = TYPE_MOD_NONE,
            .qualifiers = 0,
        };
        // We use `parser->data` outside of function calls, hence the reset.
        parser->data = &pointer;
        type_parser_parse(parser, state, recurse + 1);
        parser->data = pointer.pointee;
        
        // Return, don't break as we don't want to print the lone pointee.
        return;
    }
    case TYPE_TOKEN_EOF:
        break;
    case TYPE_TOKEN_UNKNOWN:
        _parser_throw_error(parser, TYPE_PARSE_UNKNOWN);
        break;

    default:
        __builtin_assume(false);
    }
    
    if (token.type != TYPE_TOKEN_EOF)
        goto loop_start;

    _parser_finalize(parser, false);
    printf("Final Type: '");
    _parser_write(parser);
    println("'");
}
