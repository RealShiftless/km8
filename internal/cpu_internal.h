#pragma once

#include "km8/cpu.h"

typedef enum {
    EXECUTION_PENDING = 0,
    EXECUTION_FAILED,
    EXECUTION_SUCCESS
} ExecutionResult;

struct Km8Context;

void km8_cpu_step(Cpu* cpu);
typedef ExecutionResult (*Km8OpcodeFn)(struct Km8Context* ctx);

typedef struct {
    Km8OpcodeFn on_execute;
    uint8_t size;
} Km8Opcode;

uint8_t km8_cpu_alu_add(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_adc(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_sub(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_sbc(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_and(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_or(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_xor(Cpu* cpu, uint8_t a, uint8_t b);