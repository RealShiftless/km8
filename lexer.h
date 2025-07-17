#pragma once

#include "kasm.h"
#include "list.h"

typedef enum {
    LEXER_OK,
    LEXER_TOKEN_OVERFLOW,
    LEXER_TOKEN_UNKNOWN,      
    LEXER_ALLOC_FAILED,
    LEXER_STREAM_ERROR
} LexerResult;

typedef struct {
    List* tokens;

    char fileBuffer[FILE_BUFFER_SIZE];
    uint8_t fileBufferLength;

    char tokenBuffer[TOKEN_BUFFER_SIZE];
    uint8_t tokenBufferLength;

    uint32_t line;
    uint32_t position;

    uint32_t tokenPosition;

    uint8_t inComment;
    char lastChar;

    Token* errToken;
} TokenizerContext;

LexerResult lex(FILE* stream, List* tokens);

const char* get_lexer_result_msg(LexerResult result);