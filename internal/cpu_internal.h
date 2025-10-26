#pragma once

#include "km8/km8.h"
#include "bus.h"



void km8_cpu_step(Km8Context* ctx);

const Km8Opcode* km8_cpu_get_opcode(uint8_t value);

uint8_t km8_cpu_alu_add(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_adc(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_sub(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_sbc(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_and(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_or(Cpu* cpu, uint8_t a, uint8_t b);
uint8_t km8_cpu_alu_xor(Cpu* cpu, uint8_t a, uint8_t b);

// Helpers
static inline uint16_t get_addr(const uint8_t* value) {
    return (uint16_t)value[0] | ((uint16_t)value[1] << 8);
}

static inline uint8_t bus_read(Km8Context* ctx, uint16_t addr) {
    Km8BusAccess result = km8_bus_read(ctx, addr);
    ctx->cpu.latency_cycles += result.latency_cycles;
    return result.value;
}

static inline void bus_write(Km8Context* ctx, uint16_t addr, uint8_t value) {
    Km8BusAccess result = km8_bus_write(ctx, addr, value);
    ctx->cpu.latency_cycles += result.latency_cycles;
}

static inline void stack_push(Km8Context* ctx, uint8_t value) {
    uint16_t sp = km8_cpu_get_sp(&ctx->cpu);
    bus_write(ctx, sp, value);
    km8_cpu_set_sp(&ctx->cpu, sp - 1);
}

static inline uint8_t stack_pop(Km8Context* ctx) {
    uint16_t sp = km8_cpu_get_sp(&ctx->cpu);
    uint8_t value = bus_read(ctx, sp + 1);
    km8_cpu_set_sp(&ctx->cpu, sp + 1);
    return value;
}

// Helpers
static inline void km8_cpu_halt(Km8Context* ctx, CpuHaltCode haltcode) {
    ctx->cpu.halt_code = haltcode;
    ctx->cpu.state = CPU_HALT;
}