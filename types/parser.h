#pragma once

#include "types.h"
#include "lexer.h"

#include <setjmp.h>

typedef enum {
    TYPE_PARSE_NONE,
    TYPE_PARSE_UNKNOWN, // The word we parsed is not a basic type nor a modifier.
    TYPE_PARSE_INVALID, // The resulting type doesn't make sense, e.g. `long float`, `short char`.
    TYPE_PARSE_COUNT,
} Type_Parse_Error;

typedef struct {
    jmp_buf          caller;
    Type_Parse_Error error;
} Error_Handler;

typedef struct Type_Parser_Data Type_Parser_Data;
struct Type_Parser_Data {
    Type_Parser_Data    *pointee;
    Type_Base            basic;
    Type_Modifier        modifier;
    Type_Qualifier_Set   qualifiers;
};

// `Type_Info` is misleading in my opinion because this is very restrictive.
typedef struct Type_Parser Type_Parser;
struct Type_Parser {
    Type_Lexer          lexer;
    Type_Token          consumed;
    Type_Table         *table;
    Error_Handler      *handler;
    Type_Parser_Data   *data;
};

void
type_parser_parse(Type_Parser *parser, String *state, int recurse);
