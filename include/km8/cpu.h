#pragma once

#include <stdint.h>
#include <stdbool.h>

#define REGISTER_COUNT 12

#define REG_SP 7
#define REG_PC 9
#define REG_FLAG 11

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
	CPU_HALTCODE_NONE,
	CPU_HALTCODE_MANUAL,
	CPU_HALTCODE_INVALID_STATE,
	CPU_HALTCODE_INVALID_OPCODE,
	CPU_HALTCODE_INVALID_OPERAND,
	CPU_HALTCODE_TRAP,
	CPU_HALTCODE_STACK_OVERFLOW,
	CPU_HALTCODE_BUS_FAULT,
	CPU_HALTCODE_UNKNOWN
} CpuHaltCode;

typedef struct {
	uint64_t cycles;
	uint8_t latency_cycles;

	CpuState state;
	CpuHaltCode halt_code;

	uint8_t registers[REGISTER_COUNT];

	uint8_t instr_buf[8];
    uint8_t instr_len;
    uint8_t instr_pos;

	uint8_t value_buf;
	uint8_t instr_exec_cycle;
} Cpu;

uint16_t km8_cpu_get_pc(const Cpu* cpu);
uint16_t km8_cpu_set_pc(const Cpu* cpu, uint16_t addr);

uint16_t km8_cpu_get_sp(const Cpu* cpu);
uint16_t km8_cpu_set_sp(const Cpu* cpu, uint16_t addr);

uint8_t km8_cpu_get_flags(const Cpu* cpu);

bool km8_cpu_is_flag_set(const Cpu* cpu, CpuFlags flag);