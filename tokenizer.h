#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define FILE_BUFFER_SIZE 1024
#define TOKEN_BUFFER_SIZE 64

#define INITIAL_CAPACITY 64

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_LABEL_DEF,
    TOKEN_LABEL_REF,
    TOKEN_DIRECTIVE,
    TOKEN_INSTRUCTION,
    TOKEN_REGISTER,
    TOKEN_IMMEDIATE,
    TOKEN_ADDRESS,
    TOKEN_COMMA,
    TOKEN_STRING,
    TOKEN_EOL,
    TOKEN_MAX
} TokenType;

typedef enum {
    TOKENIZER_NONE,
    TOKENIZER_OK,
    TOKENIZER_TOKEN_OVERFLOW,
    TOKENIZER_TOKEN_UNKNOWN,      
    TOKENIZER_ALLOC_FAILED,
    TOKENIZER_STREAM_ERROR
} TokenizerResult;

typedef struct {
    uint8_t type;

    char value[TOKEN_BUFFER_SIZE];
    uint8_t length;

    uint32_t line;
    uint32_t position;
} Token;

typedef struct {
    Token* values;

    uint32_t length;
    uint32_t capacity;
} TokenList;

typedef struct {
    TokenList tokens;

    char fileBuffer[FILE_BUFFER_SIZE];
    uint8_t fileBufferLength;

    char tokenBuffer[TOKEN_BUFFER_SIZE];
    uint8_t tokenBufferLength;

    uint32_t line;
    uint32_t position;

    uint32_t tokenPosition;

    uint8_t inComment;
    char lastChar;

    Token errToken;
} TokenizerContext;

typedef uint8_t(*TokenHandler)(const char[TOKEN_BUFFER_SIZE], uint8_t length);

static inline Token create_eol_token(uint32_t line, uint32_t position) {
    char value[TOKEN_BUFFER_SIZE] = {
        [0] = '\n'
    };
    
    Token token = { TOKEN_EOL, value, 1, line, position };

    return token;
}

static inline Token create_comma_token(uint32_t line, uint32_t position) {
    char value[TOKEN_BUFFER_SIZE] = {
        [0] = ','
    };

    Token token = { TOKEN_COMMA, value, 1, line, position };

    return token;
}

static inline uint8_t init_token_list(TokenList* list) {
    list->values = malloc(sizeof(Token) * INITIAL_CAPACITY);

    if (list->values == NULL)
        return 1;

    list->capacity = INITIAL_CAPACITY;
    return 0;
}

uint8_t add_token(TokenList* tokens, Token token);

TokenizerResult tokenize(FILE* stream, TokenList* tokens);