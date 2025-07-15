#include "bus.h"
#include "cpu.h"
#include "opcodes.h"

static AddressRange gAddressTable[BUS_MAX] = {
	//                      Base      End        Latency  Flags
	[BUS_BIOS]         = { 0x0000,   0x00FF,     1,       RANGE_FLAG_READONLY },
	[BUS_ROM_0]        = { 0x0100,   0x40FF,     4,       RANGE_FLAG_READONLY },
	[BUS_ROM_N]        = { 0x4100,   0x80FF,     5,       RANGE_FLAG_READONLY },
	[BUS_RAM_0]        = { 0x8100,   0xA0FF,     1,       RANGE_FLAG_NONE },
	[BUS_RAM_N]        = { 0xA100,   0xC0FF,     2,       RANGE_FLAG_NONE },
	[BUS_VRAM]         = { 0xC100,   0xE0FF,     2,       RANGE_FLAG_NONE },
	[BUS_EXTERNAL_RAM] = { 0xE100,   0xF0FF,     8,       RANGE_FLAG_NONE },
	[BUS_ORAM]         = { 0xF100,   0xF1FF,     2,       RANGE_FLAG_NONE },
	[BUS_IO]           = { 0xF200,   0xF2FF,     1,       RANGE_FLAG_READONLY },
	[BUS_HRAM]         = { 0xF300,   0xF3FF,     2,       RANGE_FLAG_NONE },
	[BUS_RAM_BANK]     = { 0xF400,   0xF400,     1,       RANGE_FLAG_NONE },
	[BUS_UNUSED]       = { 0xF401,   0xFFFF,     0,       RANGE_FLAG_HALT_ON_ACCESS }
};

static uint8_t gRomBankSelect;
static uint8_t gRamBankSelect;

static uint8_t gExtRamSelect;

BusAccessResult bus_read(uint16_t address, uint8_t* value, uint8_t* latency) {
	if (address <= gAddressTable[BUS_BIOS].end) {
		*value = read_bios((address - BIOS_SIZE) & 0xFF);
		*latency = gAddressTable[BUS_BIOS].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_ROM_0].end) {
		*value = read_rom(0, (address - BIOS_SIZE) & ROM_RANGE_MASK);
		*latency = gAddressTable[BUS_ROM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_ROM_N].end) {
		*value = read_rom(gRomBankSelect, (address - BIOS_SIZE) & ROM_RANGE_MASK);
		*latency = gAddressTable[BUS_ROM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_0].end) {
		*value = read_iram(0, (address - BIOS_SIZE) & IRAM_RANGE_MASK);
		*latency = gAddressTable[BUS_RAM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_N].end) {
		*value = read_iram(gRomBankSelect, (address - BIOS_SIZE) & IRAM_RANGE_MASK);
		*latency = gAddressTable[BUS_RAM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_VRAM].end) {
		*value = 0xFF;
		*latency = gAddressTable[BUS_VRAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_EXTERNAL_RAM].end) {
		*value = read_eram(gExtRamSelect, (address - BIOS_SIZE) & ERAM_RANGE_MASK);
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
		*value = read_hram((address - BIOS_SIZE) & HRAM_RANGE_MASK);
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
		write_iram(0, (address - BIOS_SIZE) & ROM_RANGE_MASK, value);
		*latency = gAddressTable[BUS_RAM_0].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_RAM_N].end) {
		write_iram(gRomBankSelect, (address - BIOS_SIZE) & ROM_RANGE_MASK, value);
		*latency = gAddressTable[BUS_RAM_N].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_VRAM].end) {
		// Not implemented yet
		*latency = gAddressTable[BUS_VRAM].latency;
		return BUS_OK;
	}
	else if (address <= gAddressTable[BUS_EXTERNAL_RAM].end) {
		write_eram(gExtRamSelect, (address - BIOS_SIZE) & ERAM_RANGE_MASK, value);
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
		write_hram((address - BIOS_SIZE) & HRAM_RANGE_MASK, value);
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