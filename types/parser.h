#pragma once

#include "types.h"
#include "lexer.h"
#include "../strings.h"

#include <setjmp.h>

typedef struct {
    Allocator           allocator;
    CType               type;
    CType_QualifierFlag qualifiers;
    CType_BasicFlag     flags;
    jmp_buf             caller;
} CParser;

CParser
cparser_make(Allocator allocator);

/**
 * @return
 *      `true` if we successfully parsed without incident, else `false`.
 *      The error message will be printed.
 */
bool
cparser_parse(CParser *expr, CLexer *lexer);

/**
 * @brief
 *      Returns the 'canonical' type name.
 */
const char *
cparser_canonicalize(const CParser *expr, String_Builder *builder);
