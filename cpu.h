#pragma once

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "bus.h"

#define REGISTER_COUNT 12

//#define gStackPointer (*(uint16_t*)&gRegisters[REGISTER_COUNT - 5])
//#define gProgramCounter (*(uint16_t*)&gRegisters[REGISTER_COUNT - 3])
//#define gFlags (*(uint8_t*)&gRegisters[REGISTER_COUNT - 1])

#define SP_INDEX (REGISTER_COUNT - 5)
#define PC_INDEX (REGISTER_COUNT - 3)
#define FLAGS_INDEX (REGISTER_COUNT - 1)

// Enums
typedef enum {
	FLAG_C = 1 << 0,
	FLAG_Z = 1 << 1,
	FLAG_N = 1 << 2,
	FLAG_V = 1 << 3
} CpuFlags;

typedef enum {
	CPU_FETCH_OPCODE,
	CPU_DECODE,        
	CPU_FETCH_OPERANDS, 
	CPU_EXECUTE, 
	CPU_WRITEBACK,
	CPU_HALT
} CpuState;

typedef enum {
	HALTCODE_NONE,
	HALTCODE_MANUAL,
	HALTCODE_INVALID_OPCODE,
	HALTCODE_INVALID_OPERAND,
	HALTCODE_TRAP,
	HALTCODE_STACK_OVERFLOW,
	HALTCODE_BUS_FAULT,
	HALTCODE_UNKNOWN,
	HALTCODE_WRITE_PROTECTED,
	HALTCODE_READ_PROTECTED
};


// CPU State
extern uint8_t gCpuState;
extern uint8_t gCpuHaltCode;


// Registers
extern uint8_t gRegisters[REGISTER_COUNT];


// Func
void init_cpu(void);
void cpu_run_cycle(void);


// Register helpers
static inline uint16_t get_sp() {
	return gRegisters[SP_INDEX + 0] | (gRegisters[SP_INDEX + 1] << 8);
}

static inline void set_sp(uint16_t value) {
	gRegisters[SP_INDEX + 0] = value & 0xFF;
	gRegisters[SP_INDEX + 1] = value >> 8;
}

static inline void dec_sp() {
	set_sp(get_sp() - 1);
}

static inline void inc_sp() {
	set_sp(get_sp() + 1);
}

static inline uint16_t get_pc() {
	return gRegisters[PC_INDEX + 0] | (gRegisters[PC_INDEX + 1] << 8);
}
static inline void set_pc(uint16_t value) {
	gRegisters[PC_INDEX + 0] = value & 0xFF;
	gRegisters[PC_INDEX + 1] = value >> 8;
}
static inline void inc_pc() {
	set_pc(get_pc() + 1);
}

static inline uint8_t get_flags(void) {
	return gRegisters[FLAGS_INDEX];
}
static inline void clear_flags(void) {
	gRegisters[FLAGS_INDEX] = 0;
}
static inline void set_flag(uint8_t flag) {
	gRegisters[FLAGS_INDEX] |= flag;
}
static inline uint8_t is_flag_set(uint8_t flag) {
	return gRegisters[FLAGS_INDEX] & flag;
}

// Bussin helpers
static inline void set_bus_fault_halt(uint8_t result) {
	gCpuHaltCode =
		(result == BUS_HALT_READONLY) ? HALTCODE_WRITE_PROTECTED :
		(result == BUS_HALT_UNUSED) ? HALTCODE_BUS_FAULT :
		(result == BUS_HALT_INVALID) ? HALTCODE_BUS_FAULT :
		HALTCODE_UNKNOWN;

	gCpuState = CPU_HALT;
}

static inline uint8_t handle_bus_read(uint16_t address, uint8_t* value, uint8_t* latencyCycles) {
	uint8_t busResult = bus_read(address, value, latencyCycles);

	if (busResult != BUS_OK)
	{
		set_bus_fault_halt(busResult);
		return 1;
	}

	return 0;
}


static inline uint8_t handle_bus_write(uint16_t address, uint8_t value, uint8_t* latencyCycles) {
	uint8_t busResult = bus_write(address, value, latencyCycles);

	if (busResult != BUS_OK)
	{
		set_bus_fault_halt(busResult);
		return 1;
	}

	return 0;
}

