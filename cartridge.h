#pragma once

#include <stdint.h>

#define CART_ROM_BANK_S 0x4000
#define CART_RAM_BANK_S 0x2000

#define CART_RAM_MASK (CART_RAM_BANK_S - 1)

#define CART_HEADER_S 0x10

typedef enum {
	CF_NONE    = 0b00000000,
	CF_BATTERY = 0b00000001
} CartFlag;

typedef struct {
	char magic[3];
	uint8_t rom_size;
	uint8_t ram_size;
	uint8_t type_flags;

	char game_title[10];
} CartHeader;

typedef struct {
	CartHeader header;

	uint8_t** rom;
	uint8_t** ram;
} Cartridge;