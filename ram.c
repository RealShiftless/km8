#include "ram.h"

uint8_t memory[0x10000];

uint8_t MemoryGet(uint16_t address) {
	return memory[address];
}

void MemorySet(uint16_t address, uint8_t value) {
	memory[address] = value;
}