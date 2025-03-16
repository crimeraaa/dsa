#pragma once

#include "types.h"
#include "lexer.h"
#include "../strings.h"

typedef struct CParser_Handler CParser_Handler;
typedef struct CParser_Data    CParser_Data;

struct CParser_Data {
    CParser_Data       *prev;
    CType               type;
    CType_QualifierFlag qualifiers;
    CType_BasicFlag     basic_flags;
};

typedef struct {
    Allocator        allocator;
    CType_Table     *table;
    CParser_Data    *data;
    CParser_Handler *handler;
} CParser;

bool
cparser_init(CParser *parser, CType_Table *table, Allocator allocator);

/**
 * @return
 *      `true` if we successfully parsed without incident, else `false`.
 *      The error message will be printed.
 */
bool
cparser_parse(CParser *parser, CLexer *lexer);

/**
 * @brief
 *      Returns the 'canonical' type name.
 */
const char *
cparser_canonicalize(CParser *parser, String_Builder *builder);
