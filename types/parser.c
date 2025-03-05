#include "parser.h"

#include <assert.h>

static bool
_parser_data_is_integer(const Type_Parser_Data *data)
{
    return TYPE_BASE_CHAR <= data->base && data->base <= TYPE_BASE_LONG_LONG;
}

static bool
_parser_data_is_floating(const Type_Parser_Data *data)
{
    return TYPE_BASE_FLOAT <= data->base && data->base <= TYPE_BASE_LONG_DOUBLE;
}

static bool
_parser_data_is_pointer(const Type_Parser_Data *data)
{
    return data->base == TYPE_BASE_POINTER;
}

static bool
_parser_data_has_qualifier(const Type_Parser_Data *data, Type_Qualifier qualifier)
{
    return data->qualifiers & BIT(qualifier);
}

static void
_parser_throw_error(Type_Parser *parser, Type_Parse_Error error)
{
    parser->handler->error = error;
    longjmp(parser->handler->caller, 1);

}

static void
_parser_set_base(Type_Parser *parser, Type_Token token, Type_Base expected)
{
    // Was previously set?
    if (parser->data->base != TYPE_BASE_NONE) switch (parser->data->base) {
    case TYPE_BASE_SHORT:
        // Allow `short int`. Do not update `parser->data->base` because we are
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
         *      `parser->data->base` is `TYPE_BASE_LONG`, thus we would never reach
         *       here.
         */
        if (expected == TYPE_BASE_SHORT || expected == TYPE_BASE_LONG) {
            parser->data->base = expected;
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
            parser->data->base = TYPE_BASE_LONG_LONG;
            goto success;
        }
        // Allow `long double`.
        else if (expected == TYPE_BASE_DOUBLE) {
            parser->data->base = TYPE_BASE_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    case TYPE_BASE_LONG_LONG:
        // Allow `long long int`.
        if (expected == TYPE_BASE_INT)
            goto success;
        goto invalid_combination;

    case TYPE_BASE_DOUBLE:
        // Alloc `double long`.
        if (expected == TYPE_BASE_LONG) {
            parser->data->base = TYPE_BASE_LONG_DOUBLE;
            goto success;
        }
        goto invalid_combination;

    default: invalid_combination:
        printfln("ERROR: Invalid type combination " STRING_QFMTSPEC " and \'%s\'",
            string_fmtarg(token.word),
            TYPE_BASE_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
        return;
    }
    parser->data->base = expected;
success:
    printfln("Type: '%s'", TYPE_BASE_STRINGS[parser->data->base].data);
}

static void
_parser_set_modifier(Type_Parser *parser, Type_Token token, Type_Modifier expected)
{
    // You cannot repeat or combine modifiers, e.g. `signed signed int` or `signed complex float`.
    if (parser->data->modifier != TYPE_MOD_NONE) {
        printfln("ERROR: Already have modifier " STRING_QFMTSPEC " but got '%s'",
            string_fmtarg(token.word),
            TYPE_MOD_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
    }

    if (parser->data->base != TYPE_BASE_NONE) {
        switch (expected) {
        case TYPE_MOD_SIGNED: // fallthrough
        case TYPE_MOD_UNSIGNED:
            if (!_parser_data_is_integer(parser->data))
                goto invalid_combination;
            break;
        case TYPE_MOD_COMPLEX:
            if (!_parser_data_is_floating(parser->data)) invalid_combination: {
                printfln("ERROR: Invalid type and modifier '%s' and '%s'",
                    TYPE_BASE_STRINGS[parser->data->base].data,
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
    if (_parser_data_has_qualifier(parser->data, expected)) {
        printfln("ERROR: Duplicate qualifier '%s'", TYPE_QUAL_STRINGS[expected].data);
        _parser_throw_error(parser, TYPE_PARSE_INVALID);
        return;
    }
    printfln("Qualifier: '%s'", TYPE_QUAL_STRINGS[expected].data);
    parser->data->qualifiers |= BIT(expected);
}

// Ensure modifiers and qualifiers are valid for their basic types.
static void
_parser_check(Type_Parser *parser, Type_Parser_Data *data)
{
    switch (data->modifier) {
    case TYPE_MOD_SIGNED:
    case TYPE_MOD_UNSIGNED:
        if (!_parser_data_is_integer(data))
            goto invalid_combination;
        break;
    case TYPE_MOD_COMPLEX:
        if (!_parser_data_is_floating(data)) invalid_combination: {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_MOD_STRINGS[data->modifier].data,
                TYPE_BASE_STRINGS[data->base].data);
            _parser_throw_error(parser, TYPE_PARSE_INVALID);
            return;
        }
        break;
    default:
        break;
    }

    if (_parser_data_has_qualifier(data, TYPE_QUAL_RESTRICT)) {
        if (!_parser_data_is_pointer(data)) {
            printfln("ERROR: '%s' cannot be used with '%s'",
                TYPE_QUAL_STRINGS[TYPE_QUAL_RESTRICT].data,
                TYPE_BASE_STRINGS[data->base].data);
            _parser_throw_error(parser, TYPE_PARSE_INVALID);
            return;
        }
    }
    return;
}

static void
_parser_finalize(Type_Parser *parser, Type_Parser_Data *data, bool is_pointee)
{
    // Map default types for lone modifiers.
    if (data->base == TYPE_BASE_NONE) {
        switch (data->modifier) {
        // `signed` maps to `signed int` which in turn maps to `int`.
        case TYPE_MOD_SIGNED:
        // `unsigned` maps to `unsigned int`.
        case TYPE_MOD_UNSIGNED:
            data->base = TYPE_BASE_INT;
            break;

        // `complex` maps to `complex double`.
        case TYPE_MOD_COMPLEX:
            data->base = TYPE_BASE_DOUBLE;
            break;

        default:
            // Only throw errors for the innermost recursive call.
            if (!is_pointee) {
                println("ERROR: No basic type nor modifier were received.");
                _parser_throw_error(parser, TYPE_PARSE_INVALID);
            }
            break;
        }
    }

    // Ensure all the integer types (sans `char`) are signed when not specified.
    switch (data->base) {
    case TYPE_BASE_SHORT:
    case TYPE_BASE_INT:
    case TYPE_BASE_LONG:
    case TYPE_BASE_LONG_LONG:
        if (data->modifier == TYPE_MOD_NONE)
            data->modifier = TYPE_MOD_SIGNED;
        break;

    default:
        break;
    }
    _parser_check(parser, data);
}

static void
_parser_write_qualifier(const Type_Parser_Data *data, String_Builder *builder, Type_Qualifier qualifier)
{
    if (data->qualifiers & BIT(qualifier)) {
        string_append_string(builder, TYPE_QUAL_STRINGS[qualifier]);
        string_append_char(builder, ' ');
    }
}

static void
_parser_try_write_qualifiers(const Type_Parser_Data *data, String_Builder *builder)
{
    _parser_write_qualifier(data, builder, TYPE_QUAL_CONST);
    _parser_write_qualifier(data, builder, TYPE_QUAL_VOLATILE);
    _parser_write_qualifier(data, builder, TYPE_QUAL_RESTRICT);
}

static const char *
_parser_write(const Type_Parser_Data *data, String_Builder *builder)
{
    if (data->pointee != NULL) {
        // This is just an intermediate result, we don't care about it.
        _parser_write(data->pointee, builder);
    }

    if (!_parser_data_is_pointer(data))
        _parser_try_write_qualifiers(data, builder);

    switch (data->modifier) {
    case TYPE_MOD_NONE: write_type_only:
        string_append_string(builder, TYPE_BASE_STRINGS[data->base]);
        break;
    case TYPE_MOD_SIGNED:
        if (data->base != TYPE_BASE_CHAR)
            goto write_type_only;
        // Fallthrough
    case TYPE_MOD_UNSIGNED:
    case TYPE_MOD_COMPLEX:
        string_append_string(builder, TYPE_MOD_STRINGS[data->modifier]);
        string_append_char(builder, ' ');
        string_append_string(builder, TYPE_BASE_STRINGS[data->base]);
        break;
    default:
        break;
    }

    if (_parser_data_is_pointer(data)) {
        string_append_cstring(builder, " *");
        _parser_try_write_qualifiers(data, builder);
    }
    return string_to_cstring(builder);
}

void
type_parser_parse(Type_Parser *parser, int recurse)
{
    Type_Token     token;
    int            steps   = 1;
    String_Builder builder = string_builder_make(parser->allocator);

loop_start:
    // Ugly to use goto like this but it works for our purposes...
    token = type_lexer_scan(&parser->lexer);
    for (int i = 0; i < recurse; ++i) {
        fputc('\t', stdout);
    }
    printf("[%i] ", steps++);

    switch (token.type) {
    // Integer
    case TYPE_TOKEN_CHAR:   _parser_set_base(parser, token, TYPE_BASE_CHAR);    break;
    case TYPE_TOKEN_SHORT:  _parser_set_base(parser, token, TYPE_BASE_SHORT);   break;
    case TYPE_TOKEN_INT:    _parser_set_base(parser, token, TYPE_BASE_INT);     break;
    case TYPE_TOKEN_LONG:   _parser_set_base(parser, token, TYPE_BASE_LONG);    break;

    // Float
    case TYPE_TOKEN_FLOAT:  _parser_set_base(parser, token, TYPE_BASE_FLOAT);   break;
    case TYPE_TOKEN_DOUBLE: _parser_set_base(parser, token, TYPE_BASE_DOUBLE);  break;

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

    case TYPE_TOKEN_VOID:       _parser_set_base(parser, token, TYPE_BASE_VOID); break;
    case TYPE_TOKEN_ASTERISK: {
        Allocator_Error   error;
        Type_Parser_Data *pointer = mem_new(Type_Parser_Data, &error, parser->allocator);
        if (error != ALLOCATOR_ERROR_NONE)
            _parser_throw_error(parser, TYPE_PARSE_INVALID);

        pointer->pointee    = parser->data;
        pointer->base       = TYPE_BASE_POINTER;
        pointer->modifier   = TYPE_MOD_NONE;
        pointer->qualifiers = 0;

        parser->data = pointer;
        _parser_finalize(parser, pointer->pointee, true);
        
        printfln("Pointer to: '%s'", _parser_write(pointer->pointee, &builder));
        string_builder_reset(&builder);
        
        // This will continue the loop.
        break;
    }
    case TYPE_TOKEN_EOF:
        break;
    case TYPE_TOKEN_UNKNOWN:
        printfln("[ERROR]: Unknown token " STRING_QFMTSPEC, string_fmtarg(token.word));
        _parser_throw_error(parser, TYPE_PARSE_UNKNOWN);
        break;

    default:
        assert(false);
    }
    
    if (token.type != TYPE_TOKEN_EOF)
        goto loop_start;

    _parser_finalize(parser, parser->data, false);
    printfln("Final Type: '%s'", _parser_write(parser->data, &builder));
    string_builder_destroy(&builder);
}
