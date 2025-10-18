#include "cpu_internal.h"

#include "km8/km8.h"
#include "bus.h"

#define KM8_VALIDATE_REGISTER(reg) \
    do {                           \
        if ((reg) >= REGISTER_COUNT) \
            return EXECUTION_FAILED; \
    } while (0)

#define OPCODE(func, size) ((Km8Opcode) {func, size})


// Helpers
static inline uint16_t get_addr(const uint8_t* value) {
    return value[0] | (value[1] << 8);
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


// Opcode Func
static ExecutionResult op_nop(Km8Context* ctx) {
    return EXECUTION_SUCCESS;
}

// Args: reg, mem_low, mem_high
static ExecutionResult op_ldr_mem(Km8Context* ctx) {
    switch(ctx->cpu.instr_exec_cycle) {
        case 0: {
            uint16_t addr = get_addr(ctx->cpu.instr_buf + 2);
            ctx->cpu.value_buf = bus_read(ctx, addr);
            return EXECUTION_PENDING;
        }
            
        case 1: {
            uint8_t reg = ctx->cpu.instr_buf[1];
            KM8_VALIDATE_REGISTER(reg);

            ctx->cpu.registers[reg] = ctx->cpu.value_buf;
            return EXECUTION_SUCCESS;
        }
            
        default: return EXECUTION_FAILED;
    }
}

// Args: reg, imm
static ExecutionResult op_ldr_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    ctx->cpu.value_buf = ctx->cpu.instr_buf[2];

    ctx->cpu.registers[reg] = ctx->cpu.value_buf;
    return EXECUTION_SUCCESS;
}

// Args: reg, mem_low, mem_high
static ExecutionResult op_str(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint16_t addr = get_addr(ctx->cpu.instr_buf + 2);

    bus_write(ctx, addr, value);

    return EXECUTION_SUCCESS;
}

// Args: reg1, reg2
static ExecutionResult op_mov(Km8Context* ctx) {
    uint8_t reg1 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg1);

    uint8_t reg2 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg2);

    ctx->cpu.registers[reg1] = ctx->cpu.registers[reg2];

    return EXECUTION_SUCCESS;
}

// Args: reg1, reg2
static ExecutionResult op_swp(Km8Context* ctx) {
    uint8_t reg1 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg1);

    uint8_t reg2 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg2);

    uint8_t temp = ctx->cpu.registers[reg1];
    ctx->cpu.registers[reg1] = ctx->cpu.registers[reg2];
    ctx->cpu.registers[reg2] = temp;

    return EXECUTION_SUCCESS; 
}

// Args: reg
static ExecutionResult op_push(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    stack_push(ctx, value);

    return EXECUTION_SUCCESS;
}

// Args: reg
static ExecutionResult op_pop(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = stack_pop(ctx);
    ctx->cpu.registers[reg] = value;

    return EXECUTION_SUCCESS;
}

// Args: reg
static ExecutionResult op_clr(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    ctx->cpu.registers[reg] = 0;

    return EXECUTION_SUCCESS;
}


static const Km8Opcode gOpcodeTable[256] = {
    //              Func         Size
    [0x00] = OPCODE(op_nop,      1),
    [0x01] = OPCODE(op_ldr_mem,  4),
    [0x02] = OPCODE(op_ldr_imm,  3),
    [0x03] = OPCODE(op_str,      4),
    [0x04] = OPCODE(op_mov,      3),
    [0x05] = OPCODE(op_swp,      3),
    [0x06] = OPCODE(op_push,     2),
    [0x07] = OPCODE(op_pop,      2),
    [0x08] = OPCODE(op_clr,      2)
};
