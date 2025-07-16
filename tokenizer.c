#include "tokenizer.h"
#include "opcodes.h"

// Checks if the token is a label definition (e.g., "@start:")
static uint8_t parse_label_def(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '@' && token[length - 1] == ':');
}

// Checks if the token is a label reference (e.g., "@start")
static uint8_t parse_label_ref(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '@');
}

// Checks if the token is a directive (e.g., ".data", ".text")
static uint8_t parse_directive(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '.');
}

// Checks if the token is a valid instruction (matches an opcode)
static uint8_t parse_instruction(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    for (int i = 0; i < MAX_OPCODES; i++) {
        if (strncmp(token, gOpcodes[i].mnemonic, length) == 0)
            return 1;
    }
    return 0;
}

// Checks if the token is a valid register (e.g., r0, r1, ..., r5)
static uint8_t parse_register(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    if (token[0] != 'r' || length != 2)
        return 0;

    return (token[1] >= '0' && token[1] <= '5');
}

// Checks if the token represents an immediate value (e.g., #10, #0xFF)
static uint8_t parse_immediate(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '#');
}

// Checks if the token represents an address (e.g., $1000, $FF)
static uint8_t parse_address(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '$');
}

// Checks if the token is a comma (used to separate operands)
static uint8_t parse_comma(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == ',' && length == 1);
}

// Checks if the token is an end-of-line (newline character '\n')
static uint8_t parse_eol(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '\n' && length == 1);
}

// Parse string
static uint8_t parse_string(const char token[TOKEN_BUFFER_SIZE], uint8_t length) {
    return (token[0] == '"' && token[length - 1] == '"');
}


// Parse Table
static TokenHandler token_handlers[] = {
    [TOKEN_LABEL_DEF]   = parse_label_def,   // Handler for label definitions (e.g., "@start:")
    [TOKEN_LABEL_REF]   = parse_label_ref,   // Handler for label references (e.g., "@start")
    [TOKEN_DIRECTIVE]   = parse_directive,   // Handler for directives (e.g., ".data", ".text")
    [TOKEN_INSTRUCTION] = parse_instruction, // Handler for instructions (e.g., "MOV", "ADD")
    [TOKEN_REGISTER]    = parse_register,    // Handler for registers (e.g., "r0", "r1")
    [TOKEN_IMMEDIATE]   = parse_immediate,   // Handler for immediate values (e.g., "#10", "#0xFF")
    [TOKEN_ADDRESS]     = parse_address,     // Handler for addresses (e.g., "$1000", "$FF")
    [TOKEN_COMMA]       = parse_comma,       // Handler for commas (e.g., separating operands)
    [TOKEN_STRING]      = parse_string,
    [TOKEN_EOL]         = parse_eol,         // Handler for end-of-line (newline)
};

uint8_t add_token(TokenList* list, Token token) {
    if (list->values == NULL)
        return 1;

    if (list->length + 1 >= list->capacity) {
        list->capacity *= 2;  // Double the capacity
        list->values = realloc(list->values, list->capacity * sizeof(Token));  // Resize the array

        if (list->values == NULL)
            return 1;  // Error
    }

    list->values[list->length++] = token;
    return 0;
}

uint8_t parse_token_type(const char value[TOKEN_BUFFER_SIZE], uint8_t length, TokenType* tokenType) {
    for (uint8_t i = 0; i < TOKEN_MAX; i++) {
        if (!token_handlers[i](value, length))
            continue;

        *tokenType = i;
        return 0;
    }
    return 1;
}

TokenizerResult parse_token(TokenizerContext* context) {
    Token token = { 0 };
    
    token.line = context->line;
    token.position = context->tokenPosition;
    token.length = context->tokenBufferLength;
    memcpy(token.value, context->tokenBuffer, token.length);

    if (parse_token_type(context->tokenBuffer, context->fileBufferLength, &context->tokens))
    {
        context->errToken = token;
        return TOKENIZER_TOKEN_UNKNOWN;
    }

    if (add_token(&context->tokens, token))
        return TOKENIZER_ALLOC_FAILED;

    memset(context->tokenBuffer, 0, TOKEN_BUFFER_SIZE);
    context->tokenBufferLength = 0;

    return TOKENIZER_OK;
}

TokenizerResult tokenize_buffer(TokenizerContext* context) {
    for (int i = 0; i < context->fileBufferLength; i++) {
        TokenizerResult result = TOKENIZER_OK;
        switch (context->fileBuffer[i]) {
            case ';':
                context->inComment = 1;
                
                if (context->tokenBufferLength == 0)
                    break;

                result = parse_token(context);
                break;

            case ' ':
                if (context->tokenBufferLength == 0)
                    break;

                if ((result = parse_token(context)) != TOKENIZER_OK)
                    break;

                context->tokenPosition = context->position + 1;
                context->position++;
                break;

            case '\n':
                if (context->inComment) {
                    context->inComment = 0;
                }
                else if (context->tokenBufferLength > 0) {
                    TokenizerResult result;
                    if ((result = parse_token(context)) != TOKENIZER_OK)
                        break;
                }

                add_tokens(&context->tokens, create_eol_token(context->line, context->position));
                context->line++;
                context->tokenPosition = 0;
                break;

            case ',':
                if (context->tokenBufferLength > 0) {
                    if ((result = parse_token(context)) != TOKENIZER_OK)
                        break;
                }

                add_tokens(&context->tokens, create_comma_token(context->line, context->position));
                context->tokenPosition = context->position + 1;
                context->position++;
                break;

            default:
                if (context->inComment)
                    break;

                if (context->tokenBufferLength + 1 >= TOKEN_BUFFER_SIZE) {
                    result = TOKENIZER_TOKEN_OVERFLOW;
                    break;
                }

                context->tokenBuffer[context->tokenBufferLength++] = (char)i;
                context->position++;
                break;
        }

        // Return if an error
        if (result != TOKENIZER_OK)
            return result;
    }

    return TOKENIZER_OK;
}

TokenizerResult tokenize(FILE* stream, TokenList* tokens) {
    TokenizerContext context = { 0 };
    init_token_list(&context.tokens);

    size_t bytesRead;
    while ((bytesRead = fread(context.fileBuffer, 1, FILE_BUFFER_SIZE, stream)) > 0) {
        TokenizerResult result;

        if((result = tokenize_buffer(&context)) != TOKENIZER_OK)
            return result;
    }

    if (ferror(stream))
        return TOKENIZER_STREAM_ERROR;
    
    return TOKENIZER_OK;
}