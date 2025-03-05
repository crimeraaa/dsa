#include "lexer.h"
#include "../ascii.h"

Type_Lexer
type_lexer_make(const char *text, size_t len)
{
    Type_Lexer lexer = {
        .start   = text,
        .current = text,
        .end     = text + len,
    };
    return lexer;
}

static char
peek(const Type_Lexer *lexer)
{
    return *lexer->current;
}

static char
advance(Type_Lexer *lexer)
{
    return *lexer->current++;
}

static void
skip_whitespace(Type_Lexer *lexer)
{
    for (;;) {
        if (ascii_is_whitespace(peek(lexer))) {
            advance(lexer);
            continue;
        }
        break;
    }
}

static Type_Token
make_token(const Type_Lexer *lexer, Type_Token_Type type)
{
    Type_Token token = {
        .type = type,
        .word = {.data = lexer->start, .len = cast(size_t)(lexer->current - lexer->start)},
    };
    return token;
}

#define lit     string_literal

const String
TYPE_TOKEN_STRINGS[TYPE_TOKEN_COUNT] = {
    // Integers (sans `long long`)
    [TYPE_TOKEN_CHAR]       = lit("char"),
    [TYPE_TOKEN_SHORT]      = lit("short"),
    [TYPE_TOKEN_INT]        = lit("int"),
    [TYPE_TOKEN_LONG]       = lit("long"),
    
    // Floating-point (sans `long double`)
    [TYPE_TOKEN_FLOAT]      = lit("float"),
    [TYPE_TOKEN_DOUBLE]     = lit("double"),

    // User-defined
    [TYPE_TOKEN_STRUCT]     = lit("struct"),
    [TYPE_TOKEN_ENUM]       = lit("enum"),
    [TYPE_TOKEN_UNION]      = lit("union"),
    [TYPE_TOKEN_IDENT]      = lit("<identifier>"),
    
    // Modifiers
    [TYPE_TOKEN_SIGNED]     = lit("signed"),
    [TYPE_TOKEN_UNSIGNED]   = lit("unsigned"),
    [TYPE_TOKEN_COMPLEX]    = lit("complex"),
    
    // Qualifiers
    [TYPE_TOKEN_CONST]      = lit("const"),
    [TYPE_TOKEN_VOLATILE]   = lit("volatile"),
    [TYPE_TOKEN_RESTRICT]   = lit("restrict"),
    
    // Misc.
    [TYPE_TOKEN_VOID]       = lit("void"),
    [TYPE_TOKEN_ASTERISK]   = lit("<pointer>"),
    [TYPE_TOKEN_EOF]        = lit("<eof>"),
    [TYPE_TOKEN_UNKNOWN]    = lit("<unknown>"),
};

#undef lit

static Type_Token
set_reserved_or_ident(const Type_Lexer *lexer, String word, Type_Token_Type type)
{
    if (string_eq(word, TYPE_TOKEN_STRINGS[type])) {
        return make_token(lexer, type);
    }
    return make_token(lexer, TYPE_TOKEN_IDENT);
}

static Type_Token
make_reserved_or_ident(const Type_Lexer *lexer)
{
    String word = {lexer->start, cast(size_t)(lexer->current - lexer->start)};
    switch (word.data[0]) {
    case 'c':
        switch (word.len) {
        case 4: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_CHAR);
        case 5: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_CONST);
        case 7: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_COMPLEX);
        }
        break;
    case 'd': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_DOUBLE);
    case 'e': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_ENUM);
    case 'f': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_FLOAT);
    case 'i': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_INT);
    case 'l': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_LONG);
    case 'r': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_RESTRICT);
    case 's':
        if (word.len < 5)
            break;
        switch (word.data[1]) {
        case 'h': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_SHORT);
        case 'i': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_SIGNED);
        case 't': return set_reserved_or_ident(lexer, word, TYPE_TOKEN_STRUCT);
        }
        break;
    case 'u':
        switch (word.len) {
        case 5: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_UNION);
        case 8: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_UNSIGNED);
        }
        break;
    case 'v':
        switch (word.len) {
        case 4: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_VOID);
        case 8: return set_reserved_or_ident(lexer, word, TYPE_TOKEN_VOLATILE);
        }
        break;
    default:
        break;
    }
    return make_token(lexer, TYPE_TOKEN_IDENT);
}

Type_Token
type_lexer_scan(Type_Lexer *lexer)
{
    skip_whitespace(lexer);
    lexer->start = lexer->current;
    if (lexer->current >= lexer->end)
        return make_token(lexer, TYPE_TOKEN_EOF);

    
    char ch = advance(lexer);
    if (ascii_is_alpha(ch)) {
        advance(lexer);
        while (ascii_is_alnum(peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }
        return make_reserved_or_ident(lexer);
    }
    return make_token(lexer, (ch == '*') ? TYPE_TOKEN_ASTERISK : TYPE_TOKEN_UNKNOWN);
}
