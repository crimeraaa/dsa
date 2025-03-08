#pragma once

#include "types.h"

typedef struct C_Lexer C_Lexer;
struct C_Lexer {
    const char *start;      // The start of this lexeme into the source text.
    const char *current;    // Our current cursor into the source text.
    const char *end;        // 1 past the last valid character in the source text.
};

enum C_TokenType {
    C_TokenType_Invalid,

    // Boolean
    C_TokenType_Bool,

    // Integer (sans `long long`)
    C_TokenType_Char, C_TokenType_Short, C_TokenType_Int, C_TokenType_Long,

    // Floating-point (sans `long double`)
    C_TokenType_Float, C_TokenType_Double,

    // User-defined
    C_TokenType_Struct, C_TokenType_Enum, C_TokenType_Union, C_TokenType_Ident,

    // Modifiers
    C_TokenType_Signed, C_TokenType_Unsigned, C_TokenType_Complex,

    // Qualifiers
    C_TokenType_Const, C_TokenType_Volatile, C_TokenType_Restrict,

    // Misc.
    C_TokenType_Void, C_TokenType_Asterisk, C_TokenType_Eof,

    C_TokenType_Count,
};
typedef enum C_TokenType C_TokenType;

extern const String
c_token_strings[C_TokenType_Count];

typedef struct C_Token C_Token;
struct C_Token {
    C_TokenType type;
    String      word;
};

C_Lexer
c_lexer_make(const char *text, size_t len);

C_Token
c_lexer_scan(C_Lexer *lexer);
