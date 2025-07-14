#include "ram.h"

uint8_t gStaticRam[0x2000];
uint8_t gBankedRam[4][0x4000];

uint8_t MemoryGet(uint8_t bank, uint16_t address) {
	return gBankedRam[bank][address];
}

void MemorySet(uint8_t bank, uint16_t address, uint8_t value) {
	gBankedRam[bank][address] = value;
}