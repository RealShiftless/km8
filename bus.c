#include "bus.h"

static AddressRange gAddressTable[BUS_MAX] = {
	//					   Base    End     Latency
	[BUS_ROM_0] =		 { 0x0000, 0x3FFF, 4, RANGE_FLAG_READONLY },
	[BUS_ROM_N] =		 { 0x4000, 0x7FFF, 5, RANGE_FLAG_READONLY },
	[BUS_RAM_0] =		 { 0x8000, 0x9FFF, 1, RANGE_FLAG_NONE },
	[BUS_RAM_N] =		 { 0xA000, 0xBFFF, 2, RANGE_FLAG_NONE },
	[BUS_VRAM] =		 { 0xC000, 0xDFFF, 2, RANGE_FLAG_NONE },
	[BUS_EXTERNAL_RAM] = { 0xE000, 0xEFFF, 8, RANGE_FLAG_NONE },
	[BUS_ORAM] =		 { 0xF000, 0xF0FF, 2, RANGE_FLAG_NONE },
	[BUS_IO] =			 { 0xF100, 0xF1FF, 1, RANGE_FLAG_READONLY },
	[BUS_HRAM] =		 { 0xF200, 0xF2FF, 2, RANGE_FLAG_NONE },
	[BUS_RAM_BANK] =     { 0xF300, 0xF300, 1, RANGE_FLAG_NONE },
	[BUS_UNUSED] =		 { 0xF301, 0xFEFF, 0, RANGE_FLAG_HALT_ON_ACCESS },
	[BUS_BIOS] =		 { 0xFF00, 0xFFFF, 1, RANGE_FLAG_READONLY }
};

static uint8_t gRomBankSelect;
static uint8_t gRamBankSelect;

static uint8_t gExtRamSelect;

BusAccessResult bus_read(uint16_t address, uint8_t* value, uint8_t* latency) {
	if (address <= gAddressTable[BUS_ROM_0].end) {
		*value = read_rom(0, address & IRAM_RANGE_MASK);
		*latency = gAddressTable[BUS_ROM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_ROM_N].end) {
		*value = read_rom(gRomBankSelect, address & IRAM_RANGE_MASK);
		*latency = gAddressTable[BUS_ROM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_0].end) {
		*value = read_iram(0, address & ROM_RANGE_MASK);
		*latency = gAddressTable[BUS_RAM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_N].end) {
		*value = read_iram(gRomBankSelect, address & ROM_RANGE_MASK);
		*latency = gAddressTable[BUS_RAM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_VRAM].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_VRAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_EXTERNAL_RAM].end) {
		*value = read_eram(gExtRamSelect, address & ERAM_RANGE_MASK);
		*latency = gAddressTable[BUS_EXTERNAL_RAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_ORAM].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_ORAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_IO].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_IO].latency;
		return BUS_OK; 
	}
	else if (address <= gAddressTable[BUS_HRAM].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_HRAM].latency;
		return BUS_OK;
	}
	else if (address == gAddressTable[BUS_RAM_BANK].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_RAM_BANK].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_UNUSED].end) {
		return BUS_HALT_UNUSED;
	}
	else if (address <= gAddressTable[BUS_BIOS].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_BIOS].latency;
		return BUS_OK;
	}

	// Fallback — truly invalid range
	*value = 0xFF;
	*latency = 0;
	return BUS_HALT_INVALID;
}

BusAccessResult bus_write(uint16_t address, uint8_t value, uint8_t* latency) {
	if (address <= gAddressTable[BUS_ROM_0].end) {
		return BUS_HALT_READONLY;
	}
	else if (address <= gAddressTable[BUS_ROM_N].end) {
		return BUS_HALT_READONLY;
	}
	else if (address <= gAddressTable[BUS_RAM_0].end) {
		write_iram(0, address & ROM_RANGE_MASK, value);
		*latency = gAddressTable[BUS_RAM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_N].end) {
		write_iram(gRomBankSelect, address & ROM_RANGE_MASK, value);
		*latency = gAddressTable[BUS_RAM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_VRAM].end) {
		// Not implemented yet
		*latency = gAddressTable[BUS_VRAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_EXTERNAL_RAM].end) {
		write_eram(gExtRamSelect, address & ERAM_RANGE_MASK, value);
		*latency = gAddressTable[BUS_EXTERNAL_RAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_ORAM].end) {
		// Not implemented yet
		*latency = gAddressTable[BUS_ORAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_IO].end) {
		// Possibly write-protected IO (e.g. read-only registers)
		return BUS_HALT_READONLY;
	}
	else if (address <= gAddressTable[BUS_HRAM].end) {
		// Not implemented yet
		*latency = gAddressTable[BUS_HRAM].latency;
		return BUS_OK;
	}
	else if (address == gAddressTable[BUS_RAM_BANK].end) {
		gRamBankSelect = value & 0x0F; // or however many banks you support
		*latency = gAddressTable[BUS_RAM_BANK].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_UNUSED].end) {
		*latency = gAddressTable[BUS_UNUSED].latency;
		return BUS_HALT_UNUSED;
	}
	else if (address <= gAddressTable[BUS_BIOS].end) {
		return BUS_HALT_READONLY;
	}

	// Out of range fallback
	*latency = 0;
	return BUS_HALT_INVALID;
}