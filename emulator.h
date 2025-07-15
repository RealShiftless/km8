#include <stdint.h>
#include "cartridge.h"

#define RAM_BANKS 4
#define RAM_BANK_SIZE 0x4000

#define HRAM_SIZE 256

#define CYCLES_PER_FRAME 200000
#define BIOS_SIZE 256


typedef enum {
	EMU_NONE       = 0b00000000,
	EMU_RUNNING    = 0b00000001,
	EMU_ROM_LOADED = 0b00000010
} EmulatorFlags;

typedef enum {
	EXEC_IDLE,
	EXEC_NORMAL,
	EXEC_PAUSED,
	EXEC_STEPPED,
} ExecutionMode;

typedef enum {
	CTRL_READ = 0b00000001,
	CTRL_WRITE = 0b00000010
} ControlMode;

typedef enum {
	LOAD_SUCCESS,
	LOAD_FAILED,
	LOAD_INVALLID_HEADER,
	LOAD_ALLOC_FAILED
};

extern Cartridge gCartridge;

//extern uint8_t gEmulatorFlags;
extern uint8_t gExecutionMode;
extern uint64_t gTotalCycles;

extern uint64_t gCurCycle;

extern uint16_t gAddressBus;
extern uint8_t gDataBus;

extern uint8_t gControlBus;

void init_emulation(void);
void emu_update_frame();
void emu_run_cycle(void);

void emu_stop(void);
void emu_halt(void);

uint8_t load_rom(const char* path);

uint8_t read_rom(uint8_t bank, uint16_t address);
uint8_t read_rom_flat(uint32_t globalAddress);

uint8_t read_bios(uint8_t address);

uint8_t read_iram(uint8_t bank, uint16_t address);
uint8_t read_iram_flat(uint32_t globalAddress);

void write_iram(uint8_t bank, uint16_t address, uint8_t value);
void write_iram_flat(uint32_t globalAddress, uint8_t value);

uint8_t read_eram(uint8_t bank, uint16_t address);
uint8_t read_eram_flat(uint32_t globalAddress);

void write_eram(uint8_t bank, uint16_t address, uint8_t value);
void write_eram_flat(uint32_t globalAddress, uint8_t value);

uint8_t read_hram(uint8_t address);
void write_hram(uint8_t address, uint8_t value);


#pragma once