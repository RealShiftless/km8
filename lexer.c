#include "lexer.h"
#include "opcodes.h"

LexerResult parse_token(TokenizerContext* context) {
    Token* token = malloc(sizeof(Token));
    
    token->line = context->line;
    token->position = context->tokenPosition;
    token->length = context->tokenBufferLength;
    
    token->value = malloc(token->length + 1);
    if (token->value == NULL) {
        return LEXER_ALLOC_FAILED;
    }

    memcpy(token->value, context->tokenBuffer, token->length);
    token->value[token->length] = '\0';

    KasmTokenType type;
    if (parse_token_type(context->tokenBuffer, context->tokenBufferLength, &type) == 1)
    {
        context->errToken = token;
        return LEXER_TOKEN_UNKNOWN;
    }

    token->type = type;

    if (list_add(context->tokens, token))
        return LEXER_ALLOC_FAILED;

    memset(context->tokenBuffer, 0, TOKEN_BUFFER_SIZE);
    context->tokenBufferLength = 0;

    return LEXER_OK;
}

LexerResult tokenize_buffer(TokenizerContext* context) {
    for (int i = 0; i < context->fileBufferLength; i++) {
        LexerResult result = LEXER_OK;
        char chr = context->fileBuffer[i];
        switch (chr) {
            case ';':
                context->inComment = 1;
                
                if (context->tokenBufferLength == 0)
                    break;

                result = parse_token(context);
                break;

            case '\t':
            case ' ':
                if (context->tokenBufferLength == 0)
                    break;

                if ((result = parse_token(context)) != LEXER_OK)
                    break;

                context->position++;
                context->tokenPosition = context->position;
                break;

            case '\n': {
                if (context->inComment) {
                    context->inComment = 0;
                }
                else if (context->tokenBufferLength > 0) {
                    LexerResult result;
                    if ((result = parse_token(context)) != LEXER_OK)
                        break;
                }

                Token* token = malloc(sizeof(Token));

                if (token == NULL) {
                    result = LEXER_ALLOC_FAILED;
                    break;
                }

                *token = create_eol_token(context->line, context->position);
                list_add(context->tokens, token);

                context->line++;
                context->position = 0;
                context->tokenPosition = 0;
                break;
            }

            case ',': {
                if (context->tokenBufferLength > 0) {
                    if ((result = parse_token(context)) != LEXER_OK)
                        break;
                }

                Token* token = malloc(sizeof(Token));

                if (token == NULL) {
                    result = LEXER_ALLOC_FAILED;
                    break;
                }

                *token = create_comma_token(context->line, context->position);
                list_add(context->tokens, token);

                context->tokenPosition = context->position + 1;
                context->position++;
                break;
            }

            default:
                if (context->inComment)
                    break;

                if (context->tokenBufferLength + 1 >= TOKEN_BUFFER_SIZE) {
                    result = LEXER_TOKEN_OVERFLOW;
                    break;
                }

                context->tokenBuffer[context->tokenBufferLength++] = chr;
                context->position++;
                break;
        }

        // Return if an error
        if (result != LEXER_OK)
            return result;
    }

    return LEXER_OK;
}

LexerResult lex(FILE* stream, List* tokens) {
    TokenizerContext context = { 0 };
    //init_token_list(&context.tokens);

    context.tokens = tokens;

    size_t bytesRead;
    while ((bytesRead = fread(context.fileBuffer, 1, FILE_BUFFER_SIZE, stream)) > 0) {
        LexerResult result;

        context.fileBufferLength = bytesRead;

        if((result = tokenize_buffer(&context)) != LEXER_OK)
            return result;
    }

    if (ferror(stream))
        return LEXER_STREAM_ERROR;
    
    return LEXER_OK;
}

const char* get_lexer_result_msg(LexerResult result) {
    switch (result) {
    case LEXER_OK:              return "OK";
    case LEXER_TOKEN_OVERFLOW:  return "Token Buffer Overflow";
    case LEXER_TOKEN_UNKNOWN:   return "Unknown Token";
    case LEXER_ALLOC_FAILED:    return "Allocation Failed";
    case LEXER_STREAM_ERROR:    return "Stream Error";
    default:                    return "???";
    }
}