#include "lexer.h"
#include "../ascii.h"

#include <string.h>

C_Lexer
c_lexer_make(const char *text, size_t len)
{
    C_Lexer lexer = {
        .start   = text,
        .current = text,
        .end     = text + len,
    };
    return lexer;
}

static char
_c_lexer_peek(const C_Lexer *lexer)
{
    return *lexer->current;
}

static char
_c_lexer_advance(C_Lexer *lexer)
{
    return *lexer->current++;
}

static void
_c_lexer_skip_whitespace(C_Lexer *lexer)
{
    for (;;) {
        if (ascii_is_whitespace(_c_lexer_peek(lexer))) {
            _c_lexer_advance(lexer);
            continue;
        }
        break;
    }
}

static C_Token
_c_lexer_make_token(const C_Lexer *lexer, C_TokenType type)
{
    String  word  = {lexer->start, cast(size_t)(lexer->current - lexer->start)};
    C_Token token = {type, word};
    return token;
}

const String
c_token_strings[C_TokenType_Count] = {
    [C_TokenType_Invalid]   = string_literal("<invalid>"),
    [C_TokenType_Bool]      = string_literal("bool"),

    // Integers (sans `long long`)
    [C_TokenType_Char]      = string_literal("char"),
    [C_TokenType_Short]     = string_literal("short"),
    [C_TokenType_Int]       = string_literal("int"),
    [C_TokenType_Long]      = string_literal("long"),

    // Floating-point (sans `long double`)
    [C_TokenType_Float]     = string_literal("float"),
    [C_TokenType_Double]    = string_literal("double"),

    // User-defined
    [C_TokenType_Struct]    = string_literal("struct"),
    [C_TokenType_Enum]      = string_literal("enum"),
    [C_TokenType_Union]     = string_literal("union"),
    [C_TokenType_Ident]     = string_literal("<identifier>"),

    // Modifiers
    [C_TokenType_Signed]    = string_literal("signed"),
    [C_TokenType_Unsigned]  = string_literal("unsigned"),
    [C_TokenType_Complex]   = string_literal("complex"),

    // Qualifiers
    [C_TokenType_Const]     = string_literal("const"),
    [C_TokenType_Volatile]  = string_literal("volatile"),
    [C_TokenType_Restrict]  = string_literal("restrict"),

    // Misc.
    [C_TokenType_Void]      = string_literal("void"),
    [C_TokenType_Asterisk]  = string_literal("<pointer>"),
    [C_TokenType_Eof]       = string_literal("<eof>"),
};

static C_Token
_c_lexer_set_reserved_or_ident(String word, C_TokenType type)
{
    C_Token token   = {C_TokenType_Ident, word};
    String  keyword = c_token_strings[type];
    // Reduce overhead from `string_eq` as we assume we are never passing in 0
    // length strings and never passing in `String`s with the same pointers.
    if (keyword.len == word.len && memcmp(word.data, keyword.data, keyword.len) == 0)
        token.type = type;
    return token;
}

static C_Token
_c_lexer_make_reserved_or_ident(const C_Lexer *lexer)
{
    String word = {lexer->start, cast(size_t)(lexer->current - lexer->start)};
    switch (word.data[0]) {
    case 'b': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Bool);
    case 'c':
        switch (word.len) {
        case 4: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Char);
        case 5: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Const);
        case 7: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Complex);
        }
        break;
    case 'd': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Double);
    case 'e': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Enum);
    case 'f': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Float);
    case 'i': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Int);
    case 'l': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Long);
    case 'r': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Restrict);
    case 's':
        if (word.len < 5)
            break;
        switch (word.data[1]) {
        case 'h': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Short);
        case 'i': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Signed);
        case 't': return _c_lexer_set_reserved_or_ident(word, C_TokenType_Struct);
        }
        break;
    case 'u':
        switch (word.len) {
        case 5: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Union);
        case 8: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Unsigned);
        }
        break;
    case 'v':
        switch (word.len) {
        case 4: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Void);
        case 8: return _c_lexer_set_reserved_or_ident(word, C_TokenType_Volatile);
        }
        break;
    case '_':
        switch (word.len) {
        case 5:
            if (memcmp(word.data, "_Bool", 5) == 0)
                return _c_lexer_make_token(lexer, C_TokenType_Bool);
            break;
        case 8:
            if (memcmp(word.data, "_Complex", 8) == 0)
                return _c_lexer_make_token(lexer, C_TokenType_Complex);
            break;
        }
        break;
    default:
        break;
    }
    return _c_lexer_make_token(lexer, C_TokenType_Ident);
}

C_Token
c_lexer_scan(C_Lexer *lexer)
{
    _c_lexer_skip_whitespace(lexer);
    lexer->start = lexer->current;

    // Exhausted the source text?
    if (lexer->current >= lexer->end)
        return _c_lexer_make_token(lexer, C_TokenType_Eof);


    char ch = _c_lexer_advance(lexer);
    if (ascii_is_alpha(ch) || ch == '_') {
        while (ascii_is_alnum(ch) || ch == '_') {
            _c_lexer_advance(lexer);
            ch = _c_lexer_peek(lexer);
        }
        return _c_lexer_make_reserved_or_ident(lexer);
    }
    return _c_lexer_make_token(lexer, (ch == '*') ? C_TokenType_Asterisk : C_TokenType_Invalid);
}
