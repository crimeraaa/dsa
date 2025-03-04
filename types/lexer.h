#pragma once

#include "types.h"

typedef struct Type_Lexer Type_Lexer;
struct Type_Lexer {
    const char *start;
    const char *current;
    const char *end;
};

enum Type_Token_Type {
    // Integer (sans `long long`)
    TYPE_TOKEN_CHAR, TYPE_TOKEN_SHORT, TYPE_TOKEN_INT, TYPE_TOKEN_LONG,

    // Floating-point (sans `long double`)
    TYPE_TOKEN_FLOAT, TYPE_TOKEN_DOUBLE,

    // User-defined
    TYPE_TOKEN_STRUCT, TYPE_TOKEN_ENUM, TYPE_TOKEN_UNION, TYPE_TOKEN_IDENT,
    
    // Modifiers
    TYPE_TOKEN_SIGNED, TYPE_TOKEN_UNSIGNED, TYPE_TOKEN_COMPLEX,

    // Qualifiers
    TYPE_TOKEN_CONST, TYPE_TOKEN_VOLATILE, TYPE_TOKEN_RESTRICT,
    
    // Misc.
    TYPE_TOKEN_VOID, TYPE_TOKEN_ASTERISK, TYPE_TOKEN_EOF, TYPE_TOKEN_UNKNOWN,
    TYPE_TOKEN_COUNT,
};
typedef enum Type_Token_Type Type_Token_Type;

extern const String
TYPE_TOKEN_STRINGS[TYPE_TOKEN_COUNT];

typedef struct Type_Token Type_Token;
struct Type_Token {
    Type_Token_Type type;
    String          word;
};

Type_Lexer
type_lexer_make(const char *text, size_t len);

Type_Token
type_lexer_scan(Type_Lexer *lexer);
