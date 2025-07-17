#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "list.h"

// Definitions
#define FILE_BUFFER_SIZE 1024
#define TOKEN_BUFFER_SIZE 64


// Lexer
typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_LABEL_DEF,
    TOKEN_DIRECTIVE,
    TOKEN_INSTRUCTION,
    TOKEN_REGISTER,
    TOKEN_IMMEDIATE,
    TOKEN_ADDRESS,
    TOKEN_LABEL_REF,
    TOKEN_COMMA,
    TOKEN_STRING,
    TOKEN_EOL,
    TOKEN_MAX
} KasmTokenType;

typedef enum {
    TOKEN_FLAG_LABEL  = 0b00000001,
    TOKEN_FLAG_ACTION = 0b00000010,
    TOKEN_FLAG_VALUE  = 0b00000100,
    TOKEN_FLAG_COMMA  = 0b00001000,
    TOKEN_FLAG_STRING = 0b00010000,
    TOKEN_FLAG_EOL    = 0b00100000
} TokenTypeFlag;

typedef uint8_t(*TokenHandler)(const char[TOKEN_BUFFER_SIZE], uint8_t length);

typedef struct {
    TokenHandler can_parse;

    uint8_t typeFlag;

    uint8_t precedingFlag;
    uint8_t succeedingFlag;
} TokenTypeDef;

typedef struct {
    uint8_t type;

    char* value;
    uint8_t length;

    uint32_t line;
    uint32_t position;
} Token;


// Lexer func
TokenTypeDef* get_token_type_def(KasmTokenType type);

uint8_t parse_token_type(const char value[TOKEN_BUFFER_SIZE], uint8_t length, KasmTokenType* tokenType);
const char* get_token_type_name(KasmTokenType type);


// Lexer helpers
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


// Parser
typedef enum {
    INSTRUCTION_TYPE_NONE,
    INSTRUCTION_TYPE_DIRECTIVE,
    INSTRUCTION_TYPE_OPCODE,
    INSTRUCTION_TYPE_LABEL_DEF,
} InstructionType;

// I use the first 4 bits in this enum as a flag for which operand type it will assemble as
// It cleanly maps to OperandType
typedef enum {
    ARGUMENT_NONE,
    ARGUMENT_IMMEDIATE = 0b00010001,
    ARGUMENT_REGISTER = 0b00100010,
    ARGUMENT_ADDRESS = 0b00110011,
    ARGUMENT_LABEL = 0b01000011
} ArgumentType;

typedef enum {
    DIRECTIVE_ORG,
    DIRECTIVE_BANK,
    DIRECTIVE_DB,
    DIRECTIVE_MAX
    //DIRECTIVE_STRING,
    //DIRECTIVE_DEFINE,
} DirectiveType;

typedef uint8_t(*DirectiveHandle)(const char[TOKEN_BUFFER_SIZE], uint8_t length);

typedef struct {
    const char* name;
    uint8_t serializeArguments;
} DirectiveTypeDef;

typedef struct {
    uint8_t type;
    char* value;
} Argument;

typedef struct {
    uint8_t type;
    char* value;

    List arguments;
} Instruction;


// Kasm
typedef enum {
    BUILD_STATE_LOAD_FILE,
    BUILD_STATE_ALLOC_TOKENS,
    BUILD_STATE_TOKENIZE,
    BUILD_STATE_PARSE_TOKENS,
    BUILD_STATE_FINALIZE
} BuildState;

typedef enum {
    BUILD_RESULT_SUCCESS,
    BUILD_RESULT_FILE_ERROR,
    BUILD_RESULT_SYNTAX_ERROR,
    BUILD_RESULT_ALLOC_FAILED,
    BUILD_RESULT_BUFFER_OVERFLOW,
    BUILD_RESULT_UNKOWN_ERROR
} BuildResult;

typedef struct {
    uint8_t assemblerState;

    uint8_t assemblerResult;
    uint8_t tokenizerResult;

    uint16_t tokenDepth;
} BuildContext;

uint8_t kasm_build(const char* input, const char* output);
