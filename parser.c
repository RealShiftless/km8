#include "parser.h"

#include "opcodes.h"

uint8_t get_argument_size(ArgumentType type) {
	switch (type & 0x0F)
	{
		case 0:  return 0; // None
		case 1:  return 1; // Immediate
		case 2:  return 1; // Register
		case 3:  return 2; // Address, Label
		default: return 0; // ?
	}
}

uint8_t validate_token_sequence(TokenTypeDef* base, TokenTypeDef* preceding, TokenTypeDef* succeeding) {
	uint8_t pAllowFlag = base->precedingFlag;
	uint8_t sAllowFlag = base->succeedingFlag;

	uint8_t pFlag = preceding->typeFlag;
	uint8_t sFlag = succeeding->typeFlag;

	return (pAllowFlag & pFlag) && (sAllowFlag & sFlag);
}

uint8_t parse_tokens(ParserContext* context) {
	Instruction currentInstruction = { 0 };

	for (int i = 0; i < context->tokens->count; i++) {
		Token* base = context->tokens->values[i];

		Token* preceding = NULL;
		TokenTypeDef* precedingType = get_token_type_def(TOKEN_EOL);
		if (i > 0) {
			preceding = context->tokens->values[i - 1];
			precedingType = preceding->type;
		}

		Token* succeeding = NULL;
		TokenTypeDef* succeedingType = get_token_type_def(TOKEN_EOL);
		if (i < context->tokens->count - 1) {
			succeeding = context->tokens->values[i + 1];
			succeedingType = succeeding->type;
		}

		if (!validate_token_sequence(base->type, precedingType, succeedingType))
			return PARSER_TOKEN_SEQUENCE_ERROR;

		// I already throw an error in the tokenizer when i encounter TOKEN_UNKOWN, so i don't have to check this here
		if (base->type <= TOKEN_LABEL_DEF) {

		}
		if (base->type <= TOKEN_INSTRUCTION)
		{
			if (currentInstruction.type != INSTRUCTION_TYPE_NONE)
				return PARSER_MULTIPLE_INSTRUCTIONS_ERROR;

			
		}
	}
}

uint8_t parse(List* tokens, List* instructions) {
	ParserContext context = { 0 };

	context.tokens = tokens;
	context.instructions = instructions;

	parse_tokens(&tokens);
}
