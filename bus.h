#include <stdint.h>
#include "emulator.h"
#include "cpu.h"
#include "cartridge.h"

#define IRAM_RANGE_MASK (RAM_BANK_SIZE - 1)
#define ROM_RANGE_MASK (CART_ROM_BANK_S - 1)
#define ERAM_RANGE_MASK (CART_RAM_BANK_S - 1)

typedef enum {
	BUS_ROM_0,
	BUS_ROM_N,
	BUS_RAM_0,
	BUS_RAM_N,
	BUS_VRAM,
	BUS_EXTERNAL_RAM,
	BUS_ORAM,
	BUS_IO,
	BUS_HRAM,
	BUS_RAM_BANK,
	BUS_UNUSED,
	BUS_BIOS,
	BUS_MAX
};

typedef enum {
	BUS_OK,
	BUS_HALT_READONLY,
	BUS_HALT_INVALID,
	BUS_HALT_UNUSED,
	BUS_HALT_FAULT
} BusAccessResult;

typedef enum {
	RANGE_FLAG_NONE			  = 0b00000000,
	RANGE_FLAG_READONLY       = 0b00000001,
	RANGE_FLAG_HALT_ON_ACCESS = 0b00000010
};

typedef struct {
	uint16_t start;
	uint16_t end;

	uint8_t latency;
	uint8_t flags;
} AddressRange;

BusAccessResult bus_read(uint16_t address, uint8_t* value, uint8_t* latency);
BusAccessResult bus_write(uint16_t address, uint8_t value, uint8_t* latency);

#pragma once
