#pragma once

#include "types.h"

typedef struct {
    const char *start;      // The start of this lexeme into the source text.
    const char *current;    // Our current cursor into the source text.
    const char *end;        // 1 past the last valid character in the source text.
} CLexer;

typedef enum {
    CTokenType_Invalid,

    // Boolean
    CTokenType_Bool,

    // Integer (sans `long long`)
    CTokenType_Char, CTokenType_Short, CTokenType_Int, CTokenType_Long,

    // Floating-point (sans `long double`)
    CTokenType_Float, CTokenType_Double,

    // User-defined
    CTokenType_Struct, CTokenType_Enum, CTokenType_Union, CTokenType_Ident,

    // Modifiers
    CTokenType_Signed, CTokenType_Unsigned, CTokenType_Complex,

    // Qualifiers
    CTokenType_Const, CTokenType_Volatile, CTokenType_Restrict,

    // Misc.
    CTokenType_Void, CTokenType_Asterisk, CTokenType_Eof,

    CTokenType_Count,
} CTokenType;

extern const String
ctoken_strings[CTokenType_Count];

typedef struct {
    CTokenType type;
    String     word;
} CToken;

CLexer
clexer_make(const char *text, size_t len);

CToken
clexer_scan(CLexer *lexer);
