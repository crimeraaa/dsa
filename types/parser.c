#include "parser.h"

#include <stdarg.h>
#include <assert.h>

CParser
cparser_make(Allocator allocator)
{
    CParser parser = {
        .allocator  = allocator,
        .type       = ctype_basic_types[CType_BasicKind_Invalid],
        .flags      = 0,
        .qualifiers = 0,
    };
    return parser;
}

__attribute__((format (printf, 2, 3), noreturn))
static void
_cparser_throw(CParser *parser, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (fmt != NULL) {
        fputs("[ERROR]: ", stdout);
        vfprintf(stdout, fmt, args);
        putc('\n', stdout);
    }
    va_end(args);
    longjmp(parser->caller, 1);
}

/**
 * @note
 *      This function is an absolute mess! But it *does* work.
 */
static void
_cparser_set_basic(CParser *parser, CType_BasicKind kind)
{
    // We don't call this function with `CType_BasicKind_Long_Long` because 'long long'
    // is not a single lexeme. The same is true for `CType_BasicKind_Long_Double`.
    assert(kind == CType_BasicKind_Bool
        || (CType_BasicKind_Char <= kind && kind <= CType_BasicKind_Long)
        || (CType_BasicKind_Float <= kind && kind <= CType_BasicKind_Double)
        || kind == CType_BasicKind_Void);

    CType type = parser->type;
    // Have a type but it's not a basic one?
    if (type.kind != CType_Kind_Invalid && type.kind != CType_Kind_Basic) have_nonbasic_type: {
        _cparser_throw(parser, "Cannot assign '%s' to '%s'",
            ctype_basic_types[kind].basic.name.data,
            ctype_kind_strings[type.kind].data);
    }

    // Haven't yet set a type?
    if (type.basic.kind == CType_Kind_Invalid) {
        type.basic.kind = kind;
        goto good_combination;
    }

    // Assumed to already have a basic type.
    switch (kind) {
    case CType_BasicKind_Bool:
        goto have_nonbasic_type;

    case CType_BasicKind_Short:
        // Allow `int short`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Short;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Int:
        // Allow `short int`, `long int` and `long long int`. Nothing to change.
        // Implicitly allows `long int long`.
        if (type.basic.kind == CType_BasicKind_Short
            || type.basic.kind == CType_BasicKind_Long
            || type.basic.kind == CType_BasicKind_Long_Long)
            break;
        goto bad_combination;

    case CType_BasicKind_Long:
        // Allow `int long`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Long;
        // Allow `long long`.
        else if (type.basic.kind == CType_BasicKind_Long)
            type.basic.kind = CType_BasicKind_Long_Long;
        // Allow `long double`.
        else if (type.basic.kind == CType_BasicKind_Double)
            type.basic.kind = CType_BasicKind_Long_Double;
        // Allow `complex long`.
        else if (type.basic.flags & CType_BasicFlag_Complex)
            type.basic.kind = CType_BasicKind_Long_Double_Complex;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Long_Long:
        // Allow `int long long`.
        if (type.basic.kind == CType_BasicKind_Int)
            type.basic.kind = CType_BasicKind_Long_Long;
        break;

    case CType_BasicKind_Double:
        // Allow `long double`.
        if (type.basic.kind == CType_BasicKind_Long)
            type.basic.kind = CType_BasicKind_Long_Double;
        else
            goto bad_combination;
        break;

    case CType_BasicKind_Void:
        goto have_nonbasic_type;

    default: bad_combination:
        _cparser_throw(parser, "Cannot combine '%s' with '%s'",
            type.basic.name.data,
            ctype_basic_types[kind].basic.name.data);
        return;
    }

    good_combination:

    // Implicitly sets `parser->type.basic.kind`.
    parser->type = ctype_basic_types[type.basic.kind];
    printfln("%s: '%s'",
        ctype_kind_strings[parser->type.kind].data,
        parser->type.basic.name.data);
}

/**
 * @note
 *      `modifier` cannot be a combination of flags.
 */
static void
_cparser_set_modifier(CParser *parser, CType_BasicFlag modifier)
{
    assert(modifier == CType_BasicFlag_Signed
        || modifier == CType_BasicFlag_Unsigned
        || modifier == CType_BasicFlag_Complex);

    const char *name;
    switch (modifier) {
    case CType_BasicFlag_Signed:   name = "signed";   break;
    case CType_BasicFlag_Unsigned: name = "unsigned"; break;
    case CType_BasicFlag_Complex:  name = "complex";  break;

    default:
        assert(false);
    }

    // Modifier is already set to something?
    // Disallows `signed signed`, `signed unsigned`, etc.
    if (parser->flags & (CType_BasicFlag_Signed | CType_BasicFlag_Unsigned | CType_BasicFlag_Complex)) {
        // Perhaps there's a better way to do this...
        const char *prev_name;
        if (parser->flags & CType_BasicFlag_Signed)
            prev_name = "signed";
        else if (parser->flags & CType_BasicFlag_Unsigned)
            prev_name = "unsigned";
        else // if (parser->flags & CType_BasicFlag_Complex)
            prev_name = "complex";
        _cparser_throw(parser, "Cannot combine modifiers '%s' and '%s'", prev_name, name);
    }

    parser->flags |= modifier;
    printfln("CType_BasicFlag: '%s'", name);
}

/**
 * @note
 *      `qualifier` cannot be a combination of flags.
 */
static void
_cparser_set_qualifier(CParser *parser, CType_QualifierFlag qualifier)
{
    assert(qualifier == CType_QualifierFlag_Const
        || qualifier == CType_QualifierFlag_Volatile
        || qualifier == CType_QualifierFlag_Restrict);

    const char *name;
    switch (qualifier) {
    case CType_QualifierFlag_Const:      name = "const";    break;
    case CType_QualifierFlag_Volatile:   name = "volatile"; break;
    case CType_QualifierFlag_Restrict:   name = "restrict"; break;

    default:
        assert(false);
    }

    // Qualifier is already set?
    // NOTE: 'const const int' is valid in C99, but I dislike it.
    if (parser->qualifiers & qualifier)
        _cparser_throw(parser, "Duplicate qualifier '%s'", name);
    parser->qualifiers |= qualifier;
    printfln("CType_QualifierFlag: '%s'", name);
}

/**
 * @brief
 *      Throws an error in the form 'Cannot use %s with %s'
 *
 * @param name A modifier or a qualifier.
 */
static void
_cparser_semantic_error(CParser *parser, const char *name)
{
    // TODO: Don't assume we can just use `type.basic`!
    String culprit = parser->type.basic.name;
    _cparser_throw(parser, "Cannot use %s with " STRING_QFMTSPEC, name, string_fmtarg(culprit));
}

/**
 * @note
 *      Assumes we don't have duplicate types, modifiers and qualifiers.
 *      Also assumes that we have at most 1 modifier.
 *      We should have thrown errors for those beforehand.
 */
static void
_cparser_check_semantics(CParser *parser)
{
    CType          *type  = &parser->type;
    CType_BasicFlag flags = parser->flags; // Very different from `type->basic.flags`!

    // No type was given?
    if (type->kind == CType_Kind_Invalid) {
        // `signed` resolves to `int`.
        if (flags & CType_BasicFlag_Signed)
            *type = ctype_basic_types[CType_BasicKind_Int];
        // `unsigned` resolves to `unsigned int`.
        else if (flags & CType_BasicFlag_Unsigned)
            *type = ctype_basic_types[CType_BasicKind_Unsigned_Int];
        // `complex` resolves to `complex double`.
        else if (flags & CType_BasicFlag_Complex)
            *type = ctype_basic_types[CType_BasicKind_Double_Complex];
        else
            _cparser_throw(parser, "No base type received.");
    }

    // Ensure modifiers have correct usage
    bool is_integer = type->basic.flags & CType_BasicFlag_Integer;
    if (flags & CType_BasicFlag_Signed) {
        if (!is_integer)
            _cparser_semantic_error(parser, "signed");

        // All other integer types are already signed.
        if (type->basic.kind == CType_BasicKind_Char)
            *type = ctype_basic_types[CType_BasicKind_Signed_Char];

    } else if (flags & CType_BasicFlag_Unsigned) {
        if (!is_integer)
            _cparser_semantic_error(parser, "unsigned");

        // Ensure we have the correct basic type data.
        switch (type->basic.kind) {
        case CType_BasicKind_Char:      *type = ctype_basic_types[CType_BasicKind_Unsigned_Char];      break;
        case CType_BasicKind_Short:     *type = ctype_basic_types[CType_BasicKind_Unsigned_Short];     break;
        case CType_BasicKind_Int:       *type = ctype_basic_types[CType_BasicKind_Unsigned_Int];       break;
        case CType_BasicKind_Long:      *type = ctype_basic_types[CType_BasicKind_Unsigned_Long];      break;
        case CType_BasicKind_Long_Long: *type = ctype_basic_types[CType_BasicKind_Unsigned_Long_Long]; break;

        // Unreachable.
        default:
            break;
        }
    }

    if (flags & CType_BasicFlag_Complex) {
        bool is_float = type->basic.flags & CType_BasicFlag_Float;
        bool is_long  = type->basic.kind == CType_BasicKind_Long;
        if (!is_float && !is_long)
            _cparser_semantic_error(parser, "complex");

        // Ensure we have the correct basic type data.
        switch (type->basic.kind) {
        case CType_BasicKind_Float:
            *type = ctype_basic_types[CType_BasicKind_Float_Complex];
            break;
        case CType_BasicKind_Double:
            *type = ctype_basic_types[CType_BasicKind_Double_Complex];
            break;
        // Allow `long complex`. `complex long` is handled by `_cparser_set_basic()`.
        case CType_BasicKind_Long:
        case CType_BasicKind_Long_Double:
            *type = ctype_basic_types[CType_BasicKind_Long_Double_Complex];
            break;

        // Unreachable.
        default:
            break;
        }
    }

    // Finally, check the qualifiers. We only really care about 'restrict'.
    if (parser->qualifiers & CType_QualifierFlag_Restrict) {
        if (type->kind != CType_Kind_Pointer)
            _cparser_semantic_error(parser, "restrict");
    }
}

bool
cparser_parse(CParser *parser, CLexer *lexer)
{
    if (setjmp(parser->caller) != 0)
        return false;

    for (;;) {
        CToken token = clexer_scan(lexer);
        if (!token.type) {
            _cparser_throw(parser, "Invalid token " STRING_QFMTSPEC ".", string_fmtarg(token.word));
            return false;
        } else if (token.type == CTokenType_Eof) {
            _cparser_check_semantics(parser);
            return true;
        }

        switch (token.type) {
        // Boolean
        case CTokenType_Bool:   _cparser_set_basic(parser, CType_BasicKind_Bool);   break;
        // Integer Types
        case CTokenType_Char:   _cparser_set_basic(parser, CType_BasicKind_Char);   break;
        case CTokenType_Short:  _cparser_set_basic(parser, CType_BasicKind_Short);  break;
        case CTokenType_Int:    _cparser_set_basic(parser, CType_BasicKind_Int);    break;
        case CTokenType_Long:   _cparser_set_basic(parser, CType_BasicKind_Long);   break;

        // Floating-Point Types
        case CTokenType_Float:  _cparser_set_basic(parser, CType_BasicKind_Float);  break;
        case CTokenType_Double: _cparser_set_basic(parser, CType_BasicKind_Double); break;

        // User-defined types
        case CTokenType_Struct:
        case CTokenType_Enum:
        case CTokenType_Union:
        case CTokenType_Ident:  goto unsupported_token;

        // Modifiers
        case CTokenType_Signed:    _cparser_set_modifier(parser, CType_BasicFlag_Signed);   break;
        case CTokenType_Unsigned:  _cparser_set_modifier(parser, CType_BasicFlag_Unsigned); break;
        case CTokenType_Complex:   _cparser_set_modifier(parser, CType_BasicFlag_Complex);  break;

        // Qualifiers
        case CTokenType_Const:     _cparser_set_qualifier(parser, CType_QualifierFlag_Const);    break;
        case CTokenType_Volatile:  _cparser_set_qualifier(parser, CType_QualifierFlag_Volatile); break;
        case CTokenType_Restrict:  _cparser_set_qualifier(parser, CType_QualifierFlag_Restrict); break;

        // Misc.
        case CTokenType_Void:      _cparser_set_basic(parser, CType_BasicKind_Void); break;
        case CTokenType_Asterisk:
            goto unsupported_token;

        default: unsupported_token:
            _cparser_throw(parser, STRING_QFMTSPEC " ('%s') is unsupported!",
                string_fmtarg(token.word),
                ctoken_strings[token.type].data);
            return false;
        }
    }

    return false;
}

const char *
cparser_canonicalize(const CParser *parser, String_Builder *builder)
{
    CType type = parser->type;

    // Qualifiers
    if (parser->qualifiers & CType_QualifierFlag_Const)
        string_append_literal(builder, "const ");
    if (parser->qualifiers & CType_QualifierFlag_Volatile)
        string_append_literal(builder, "volatile ");
    if (parser->qualifiers & CType_QualifierFlag_Restrict)
        string_append_literal(builder, "restrict ");

    // No need to print the modifiers since they're already part of the basic types.
    switch (type.kind) {
    case CType_Kind_Invalid:
        string_append_literal(builder, "<invalid>");
        break;
    case CType_Kind_Basic:
        string_append_string(builder, type.basic.name);
        break;
    case CType_Kind_Pointer:
    case CType_Kind_Struct:
    case CType_Kind_Enum:
    case CType_Kind_Union:
    default:
        string_append_literal(builder, "<unimplemented>");
        break;
    }

    return string_to_cstring(builder);
}
