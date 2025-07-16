#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 

#include "tokenizer.h"


typedef enum {
    ASSEMBLER_SUCCESS,
    ASSEMBLER_FILE_ERROR,
    ASSEMBLER_SYNTAX_ERROR,
    ASSEMBLER_ALLOC_ERROR,
    ASSEMBLER_BUFFER_OVERFLOW,
    ASSEMBLER_UNKOWN_ERROR
} AssemblerResult;

typedef enum {
    ASSEMBLER_STATE_LOAD_FILE,
    ASSEMBLER_ALLOC_TOKENS,
    ASSEMBLER_STATE_TOKENIZE,
    ASSEMBLER_PARSE_TOKENS,
    ASSEMBLER_STATE_FINALIZE
} AssemblerState;

typedef enum {
    DIRECTIVE_ORG,
    DIRECTIVE_BANK,
    DIRECTIVE_BYTES,
    DIRECTIVE_STRING,
    DIRECTIVE_DEFINE,
};

typedef struct {
    uint8_t assemblerState;

    uint8_t assemblerResult;
    uint8_t tokenizerResult;

    uint16_t tokenDepth;
} AssemblerContext;

uint8_t kasm_assemble(const char* input, const char* output);

