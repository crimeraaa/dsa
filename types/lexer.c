#include "lexer.h"
#include "../ascii.h"

#include <string.h>

CLexer
clexer_make(const char *text, size_t len)
{
    CLexer lexer = {
        .start   = text,
        .current = text,
        .end     = text + len,
    };
    return lexer;
}

static char
_clexer_peek(const CLexer *lexer)
{
    return *lexer->current;
}

static char
_clexer_advance(CLexer *lexer)
{
    return *lexer->current++;
}

static void
_clexer_skip_whitespace(CLexer *lexer)
{
    for (;;) {
        if (ascii_is_whitespace(_clexer_peek(lexer))) {
            _clexer_advance(lexer);
            continue;
        }
        break;
    }
}

static CToken
_clexer_make_token(const CLexer *lexer, CTokenType type)
{
    String word  = {lexer->start, cast(size_t)(lexer->current - lexer->start)};
    CToken token = {type, word};
    return token;
}

const String
ctoken_strings[CTokenType_Count] = {
    [CTokenType_Invalid]   = string_literal("<invalid>"),
    [CTokenType_Bool]      = string_literal("bool"),

    // Integers (sans `long long`)
    [CTokenType_Char]      = string_literal("char"),
    [CTokenType_Short]     = string_literal("short"),
    [CTokenType_Int]       = string_literal("int"),
    [CTokenType_Long]      = string_literal("long"),

    // Floating-point (sans `long double`)
    [CTokenType_Float]     = string_literal("float"),
    [CTokenType_Double]    = string_literal("double"),

    // User-defined
    [CTokenType_Struct]    = string_literal("struct"),
    [CTokenType_Enum]      = string_literal("enum"),
    [CTokenType_Union]     = string_literal("union"),
    [CTokenType_Ident]     = string_literal("<identifier>"),

    // Modifiers
    [CTokenType_Signed]    = string_literal("signed"),
    [CTokenType_Unsigned]  = string_literal("unsigned"),
    [CTokenType_Complex]   = string_literal("complex"),

    // Qualifiers
    [CTokenType_Const]     = string_literal("const"),
    [CTokenType_Volatile]  = string_literal("volatile"),
    [CTokenType_Restrict]  = string_literal("restrict"),

    // Misc.
    [CTokenType_Void]      = string_literal("void"),
    [CTokenType_Asterisk]  = string_literal("<pointer>"),
    [CTokenType_Eof]       = string_literal("<eof>"),
};

static CToken
_clexer_set_reserved_or_ident(String word, CTokenType type)
{
    CToken token   = {CTokenType_Ident, word};
    String keyword = ctoken_strings[type];
    // Reduce overhead from `string_eq` as we assume we are never passing in 0
    // length strings and never passing in `String`s with the same pointers.
    if (keyword.len == word.len && memcmp(word.data, keyword.data, keyword.len) == 0)
        token.type = type;
    return token;
}

static CToken
_clexer_make_reserved_or_ident(const CLexer *lexer)
{
    String word = {lexer->start, cast(size_t)(lexer->current - lexer->start)};
    switch (word.data[0]) {
    case 'b': return _clexer_set_reserved_or_ident(word, CTokenType_Bool);
    case 'c':
        switch (word.len) {
        case 4: return _clexer_set_reserved_or_ident(word, CTokenType_Char);
        case 5: return _clexer_set_reserved_or_ident(word, CTokenType_Const);
        case 7: return _clexer_set_reserved_or_ident(word, CTokenType_Complex);
        }
        break;
    case 'd': return _clexer_set_reserved_or_ident(word, CTokenType_Double);
    case 'e': return _clexer_set_reserved_or_ident(word, CTokenType_Enum);
    case 'f': return _clexer_set_reserved_or_ident(word, CTokenType_Float);
    case 'i': return _clexer_set_reserved_or_ident(word, CTokenType_Int);
    case 'l': return _clexer_set_reserved_or_ident(word, CTokenType_Long);
    case 'r': return _clexer_set_reserved_or_ident(word, CTokenType_Restrict);
    case 's':
        if (word.len < 5)
            break;
        switch (word.data[1]) {
        case 'h': return _clexer_set_reserved_or_ident(word, CTokenType_Short);
        case 'i': return _clexer_set_reserved_or_ident(word, CTokenType_Signed);
        case 't': return _clexer_set_reserved_or_ident(word, CTokenType_Struct);
        }
        break;
    case 'u':
        switch (word.len) {
        case 5: return _clexer_set_reserved_or_ident(word, CTokenType_Union);
        case 8: return _clexer_set_reserved_or_ident(word, CTokenType_Unsigned);
        }
        break;
    case 'v':
        switch (word.len) {
        case 4: return _clexer_set_reserved_or_ident(word, CTokenType_Void);
        case 8: return _clexer_set_reserved_or_ident(word, CTokenType_Volatile);
        }
        break;
    case '_':
        switch (word.len) {
        case 5:
            if (memcmp(word.data, "_Bool", 5) == 0)
                return _clexer_make_token(lexer, CTokenType_Bool);
            break;
        case 8:
            if (memcmp(word.data, "_Complex", 8) == 0)
                return _clexer_make_token(lexer, CTokenType_Complex);
            break;
        }
        break;
    default:
        break;
    }
    return _clexer_make_token(lexer, CTokenType_Ident);
}

CToken
clexer_scan(CLexer *lexer)
{
    _clexer_skip_whitespace(lexer);
    lexer->start = lexer->current;

    // Exhausted the source text?
    if (lexer->current >= lexer->end)
        return _clexer_make_token(lexer, CTokenType_Eof);


    char ch = _clexer_advance(lexer);
    if (ascii_is_alpha(ch) || ch == '_') {
        while (ascii_is_alnum(ch) || ch == '_') {
            _clexer_advance(lexer);
            ch = _clexer_peek(lexer);
        }
        return _clexer_make_reserved_or_ident(lexer);
    }
    return _clexer_make_token(lexer, (ch == '*') ? CTokenType_Asterisk : CTokenType_Invalid);
}
