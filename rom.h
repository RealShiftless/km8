#include <stdint.h>

#define ROM_SIZE 0x10000

typedef enum {
	LOAD_SUCCESS,
	LOAD_FAILED,
	LOAD_SIZEWARN
};

uint8_t RomRead(uint16_t address);
uint8_t RomLoad(const char* path);

#pragma once
