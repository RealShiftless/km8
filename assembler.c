#include "assembler.h"
#include "opcodes.h"

uint8_t kasm_assemble(const char* input, const char* output, AssemblerContext* context) {
    context->assemblerState = ASSEMBLER_STATE_LOAD_FILE;
    FILE* file = fopen(input, "r");
    if (!file) {
        context->assemblerResult = ASSEMBLER_FILE_ERROR;
        return 1;
    }

    // Allocate the list of tokens
    context->assemblerState = ASSEMBLER_ALLOC_TOKENS;
    TokenList tokens = { 0 };
    if (init_token_list(&tokens)) {
        context->assemblerResult = ASSEMBLER_ALLOC_ERROR;
        return 1;
    }

    // Tokenize the stream
    context->assemblerState = ASSEMBLER_STATE_TOKENIZE;
    TokenizerResult result;
    if ((context->tokenizerResult = tokenize(file, &tokens)) != TOKENIZER_OK) {
        switch (context->tokenizerResult) {
            case TOKENIZER_TOKEN_OVERFLOW:
                context->assemblerResult = ASSEMBLER_BUFFER_OVERFLOW;
                break;

            case TOKENIZER_TOKEN_UNKNOWN:
                context->assemblerResult = ASSEMBLER_SYNTAX_ERROR;
                break;

            case TOKENIZER_ALLOC_FAILED:
                context->assemblerResult = ASSEMBLER_ALLOC_ERROR;
                break;

            case TOKENIZER_STREAM_ERROR:
                context->assemblerResult = ASSEMBLER_FILE_ERROR;
                break;

            default:
                context->assemblerResult = ASSEMBLER_UNKOWN_ERROR;
                break;
        }

        free(tokens.values);
        return 1;
    }

    // Close current file, we need to open a new one for writting :)
    if (ferror(file)) {
        free(tokens.values);
        fclose(file);

        context->assemblerResult = ASSEMBLER_FILE_ERROR;
        return 1;
    }

    fclose(file);


    // Finalize
    context->assemblerState = ASSEMBLER_STATE_FINALIZE;
    free(tokens.values);
}