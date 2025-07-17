#pragma once

#include "kasm.h"
#include "list.h"

typedef enum {
	PARSER_OK,
	PARSER_TOKEN_SEQUENCE_ERROR,
	PARSER_ALLOC_FAILED,
	PARSER_MULTIPLE_INSTRUCTIONS_ERROR
} ParserResult;

typedef struct {
	List* tokens;
	List* instructions;

	List* currentLine;
} ParserContext;

uint8_t parse(List* tokens, List* instruction);
