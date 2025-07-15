#pragma once

#include <stdint.h>
#include "cpu.h"


// Definitions
#define MAX_OPCODES 256
#define MAX_MNEMONIC 4
#define MAX_OPERANDS 2
#define MAX_OPERAND_SIZE 2
#define OPERAND_BUFFER_SIZE (MAX_OPERANDS * MAX_OPERAND_SIZE)


// Enum
typedef enum {
    OPCODE_NOP = 0x00,
    OPCODE_LDR_MEM,
    OPCODE_LDR_IMM,
    OPCODE_STR,
    OPCODE_MOV,
    OPCODE_SWP,
    OPCODE_PUSH,
    OPCODE_POP,
    OPCODE_CLR,

    OPCODE_ADD_REG = 0x10,
    OPCODE_ADD_IMM,

    OPCODE_ADC_REG,
    OPCODE_ADC_IMM,

    OPCODE_INC,

    OPCODE_SUB_REG,
    OPCODE_SUB_IMM,

    OPCODE_SBC_REG,
    OPCODE_SBC_IMM,

    OPCODE_DEC,

    OPCODE_CMP_REG,
    OPCODE_CMP_IMM,

    OPCODE_AND_REG = 0x30,
    OPCODE_AND_IMM,
    OPCODE_OR_REG,
    OPCODE_OR_IMM,
    OPCODE_XOR_REG,
    OPCODE_XOR_IMM,
    OPCODE_NOT,
    OPCODE_SHL,
    OPCODE_SHR,
    OPCODE_ROL,
    OPCODE_ROR,

    OPCODE_TST_REG,
    OPCODE_TST_IMM,

    OPCODE_JMP_MEM = 0x40,
    OPCODE_JMP_REG,
    OPCODE_JZ_MEM,
    OPCODE_JZ_REG,
    OPCODE_JNZ_MEM,
    OPCODE_JNZ_REG,
    OPCODE_JC_MEM,
    OPCODE_JC_REG,
    OPCODE_JNC_MEM,
    OPCODE_JNC_REG,
    OPCODE_JN_MEM,
    OPCODE_JN_REG,
    OPCODE_JNN_MEM,
    OPCODE_JNN_REG,
    OPCODE_JV_MEM,
    OPCODE_JV_REG,
    OPCODE_JNV_MEM,
    OPCODE_JNV_REG,

    OPCODE_CALL_MEM,
    OPCODE_CALL_REG,
    OPCODE_RET,
    OPCODE_HLT,

    OPCODE_VBLK = 0xFF

} Opcode;

typedef enum {
    OPERAND_NIL = 0,
    OPERAND_IMM,
    OPERAND_REG,
    OPERAND_MEM
} OperandType;

typedef enum {
    EXECUTION_PENDING,
    EXECUTION_SUCCES,
    EXECUTION_FAILED,
    EXECUTION_HALT
} ExecutionResult;


// Structs
typedef struct {
    uint8_t type;
} Operand;

typedef struct {
    uint8_t size;
    uint8_t cycles;

    char mnemonic[MAX_MNEMONIC];
    Operand operands[MAX_OPERANDS];
} Operator;

typedef struct {
    uint8_t opcode;
    uint16_t opcode_pc;
    uint64_t opcode_cycle;
    uint8_t opcode_size;

    uint8_t operandA_type;
    uint8_t operandB_type;

    uint8_t operand_buffer[OPERAND_BUFFER_SIZE];

    uint16_t mem_value;

    uint8_t step_cycle;
    uint8_t latency_cycles;
} ExecutionContext;


// Values
extern Operator gOpcodes[256];


// Func
void init_opcodes(void);


// Helper Func
static inline uint8_t validate_reg(uint8_t reg) {
    if (reg >= REGISTER_COUNT)
    {
        gCpuHaltCode = HALTCODE_INVALID_OPERAND;
        return 1;
    }

    return 0;
}

static inline uint8_t validate_regs(uint8_t reg0, uint8_t reg1) {
    return validate_reg(reg0) || validate_reg(reg1);
}


// Stack helper func
static inline uint8_t stack_push(ExecutionContext* context, uint8_t value) {
    uint16_t sp = get_sp();

    if (handle_bus_write(sp, value, &context->latency_cycles))
        return 1;

    set_sp(sp - 1);
    return 0;
}
static inline uint8_t stack_pop(ExecutionContext* context, uint8_t* value) {
    uint16_t sp = get_sp();
    if (handle_bus_read(sp, value, &context->latency_cycles))
        return 1;

    set_sp(sp + 1);
    return 0;
}


// PC Helper Func
static inline void set_pc_bytes(uint8_t low, uint8_t high) {
    uint16_t addr = low | (high << 8);
    set_pc(addr);
}


// Math Operations, easier to do here for different versions and flags and stuff
static inline uint8_t math_add(uint8_t a, uint8_t b) {
    uint16_t result = a + b;

    clear_flags();

    // Carry if result overflowed 8 bits
    if (result > 0xFF) set_flag(FLAG_C);

    // Zero if result is 0
    if ((uint8_t)result == 0) set_flag(FLAG_Z);

    // Negative if bit 7 set
    if (result & 0x80) set_flag(FLAG_N);

    // Overflow if signs of inputs are same and sign of result differs
    if (((a ^ b) & 0x80) == 0 && ((a ^ result) & 0x80) != 0) set_flag(FLAG_V);

    return (uint8_t)result;
}

static inline uint8_t math_adc(uint8_t a, uint8_t b) {
    // Get carry flag if set
    uint8_t carry_in = is_flag_set(FLAG_C) ? 1 : 0;

    // Reset flag
    clear_flags();

    // Full 9-bit result for overflow/carry checking
    uint16_t result = a + b + carry_in;

    // Carry out of bit 7 (unsigned overflow)
    if (result > 0xFF) set_flag(FLAG_C);

    // Zero result
    if ((uint8_t)result == 0) set_flag(FLAG_Z);

    // Negative flag (sign bit)
    if (result & 0x80) set_flag(FLAG_N);

    // Overflow (signed overflow detection)
    if (((a ^ b) & 0x80) == 0 && ((a ^ result) & 0x80) != 0) set_flag(FLAG_V);

    return (uint8_t)result;
}

static inline uint8_t math_sub(uint8_t a, uint8_t b) {
    uint16_t result = a - b;

    clear_flags();

    // Carry if borrow
    if (a < b) 
        set_flag(FLAG_C);

    // Zero if 0
    if ((uint8_t)result == 0)
        set_flag(FLAG_Z);

    // Negative if bit 7
    if (result & 0x80) 
        set_flag(FLAG_N);

    // Overflow if signs of inputs are same and sign of result differs
    if (((a ^ b) & 0x80) != 0 && ((a ^ result) & 0x80) != 0) 
        set_flag(FLAG_V);

    return (uint8_t)result;
}

static inline uint8_t math_sbc(uint8_t a, uint8_t b) {
    uint8_t borrow = is_flag_set(FLAG_C) ? 1 : 0;
    uint16_t result = a - b - borrow;

    clear_flags();

    // Set Carry if borrow occurred (unsigned underflow)
    if (a < (uint16_t)b + borrow) set_flag(FLAG_C);

    // Set Zero if result is zero
    if ((uint8_t)result == 0) set_flag(FLAG_Z);

    // Set Negative if sign bit is set
    if (result & 0x80) set_flag(FLAG_N);

    // Set Overflow if signs differ and result sign is unexpected
    if (((a ^ b) & 0x80) != 0 && ((a ^ result) & 0x80) != 0) set_flag(FLAG_V);

    return (uint8_t)result;
}

static inline uint8_t math_and(uint8_t a, uint8_t b) {
    uint8_t result = a & b;

    clear_flags();

    if (result == 0) set_flag(FLAG_Z);

    return result;
}

static inline uint8_t math_or(uint8_t a, uint8_t b) {
    uint8_t result = a | b;

    clear_flags();

    if (result == 0) set_flag(FLAG_Z);

    return result;
}

static inline uint8_t math_xor(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;

    clear_flags();

    if (result == 0) set_flag(FLAG_Z);

    return result;
}


// Data & Transfer
static inline uint8_t op_nop(ExecutionContext* context) {
    return EXECUTION_SUCCES;
}


static inline uint8_t op_ldr_mem(ExecutionContext* context) {
    switch (context->step_cycle) {
        case 0: {
            uint16_t addr = context->operand_buffer[1] | (context->operand_buffer[2] << 8);
            if (handle_bus_read(addr, &context->mem_value, &context->latency_cycles))
                return EXECUTION_FAILED;

            return EXECUTION_PENDING;
        }

        case 1: {
            uint8_t reg = context->operand_buffer[0];

            if (validate_reg(reg))
                return EXECUTION_FAILED;

            gRegisters[reg] = context->mem_value;
            return EXECUTION_SUCCES;
        }

        default: return EXECUTION_FAILED;
    }
}

static inline uint8_t op_ldr_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = context->operand_buffer[1];
    return EXECUTION_SUCCES;
}

static inline uint8_t op_str(ExecutionContext* context) {
    uint16_t addr = context->operand_buffer[1] | (context->operand_buffer[2] << 8);
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    if (handle_bus_write(addr, gRegisters[reg], &context->latency_cycles))
        return EXECUTION_FAILED;

    return EXECUTION_SUCCES;
}


static inline uint8_t op_mov(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = gRegisters[reg1];
    return EXECUTION_SUCCES;
}

static inline uint8_t op_swp(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    uint8_t temp = gRegisters[reg0];
    gRegisters[reg0] = gRegisters[reg1];
    gRegisters[reg1] = temp;

    return EXECUTION_SUCCES;
}

static inline uint8_t op_push(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    if(stack_push(context, gRegisters[reg]))
        return EXECUTION_FAILED;

    return EXECUTION_SUCCES;
}
static inline uint8_t op_pop(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    if(stack_pop(context, &gRegisters[reg]))
        return EXECUTION_FAILED;

    return EXECUTION_SUCCES;
}

static inline uint8_t op_clr(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = 0;
    return EXECUTION_SUCCES;
}

// Arithmetic
static inline uint8_t op_add_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    // actually set the result
    gRegisters[reg0] = math_add(gRegisters[reg0], gRegisters[reg1]);
    return EXECUTION_SUCCES;
}
static inline uint8_t op_add_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_add(gRegisters[reg], imm);
    return EXECUTION_SUCCES;
}

static inline uint8_t op_adc_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_adc(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_adc_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_adc(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_inc(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_add(gRegisters[reg], 1);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_sub_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_sub(gRegisters[reg0], gRegisters[reg1]);
    return EXECUTION_SUCCES;
}
static inline uint8_t op_sub_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_sub(gRegisters[reg], imm);
    return EXECUTION_SUCCES;
}

static inline uint8_t op_sbc_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_sbc(gRegisters[reg0], gRegisters[reg1]);
    return EXECUTION_SUCCES;
}
static inline uint8_t op_sbc_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_sbc(gRegisters[reg], imm);
    return EXECUTION_SUCCES;
}

static inline uint8_t op_dec(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_sub(gRegisters[reg], 1);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_cmp_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    math_sub(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_cmp_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    math_sub(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

// Bitwise
static inline uint8_t op_and_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_and(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_and_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_and(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_or_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_or(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_or_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_or(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_xor_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    gRegisters[reg0] = math_xor(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_xor_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = math_xor(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_not(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = !gRegisters[reg];

    if (gRegisters[reg] == 0) set_flag(FLAG_Z);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_shl(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = gRegisters[reg] << 1;

    if (gRegisters[reg] == 0) set_flag(FLAG_Z);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_shr(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = gRegisters[reg] >> 1;

    clear_flags();
    if (gRegisters[reg] == 0) set_flag(FLAG_Z);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_rol(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = (gRegisters[reg] << 1) | (gRegisters[reg] >> 7);

    if (gRegisters[reg] == 0) set_flag(FLAG_Z);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_ror(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    gRegisters[reg] = (gRegisters[reg] >> 1) | (gRegisters[reg] << 7);

    if (gRegisters[reg] == 0) set_flag(FLAG_Z);

    return EXECUTION_SUCCES;
}

static inline uint8_t op_tst_reg(ExecutionContext* context) {
    uint8_t reg0 = context->operand_buffer[0];
    uint8_t reg1 = context->operand_buffer[1];

    if (validate_regs(reg0, reg1))
        return EXECUTION_FAILED;

    math_and(gRegisters[reg0], gRegisters[reg1]);

    return EXECUTION_SUCCES;
}
static inline uint8_t op_tst_imm(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];
    uint8_t imm = context->operand_buffer[1];

    if (validate_reg(reg))
        return EXECUTION_FAILED;

    math_and(gRegisters[reg], imm);

    return EXECUTION_SUCCES;
}

// Control flow
static inline uint8_t op_jmp_mem(ExecutionContext* context) {
    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jmp_reg(ExecutionContext* context) {
    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}

static inline uint8_t op_jz_mem(ExecutionContext* context) {
    if (!is_flag_set(FLAG_Z))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jz_reg(ExecutionContext* context) {
    if (!is_flag_set(FLAG_Z))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnz_mem(ExecutionContext* context) {
    if (is_flag_set(FLAG_Z))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnz_reg(ExecutionContext* context) {
    if (is_flag_set(FLAG_Z))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}

static inline uint8_t op_jc_mem(ExecutionContext* context) {
    if (!is_flag_set(FLAG_C))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jc_reg(ExecutionContext* context) {
    if (!is_flag_set(FLAG_C))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnc_mem(ExecutionContext* context) {
    if (is_flag_set(FLAG_C))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnc_reg(ExecutionContext* context) {
    if (is_flag_set(FLAG_C))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}

static inline uint8_t op_jn_mem(ExecutionContext* context) {
    if (!is_flag_set(FLAG_N))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jn_reg(ExecutionContext* context) {
    if (!is_flag_set(FLAG_N))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnn_mem(ExecutionContext* context) {
    if (is_flag_set(FLAG_N))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnn_reg(ExecutionContext* context) {
    if (!is_flag_set(FLAG_N))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}

static inline uint8_t op_jv_mem(ExecutionContext* context) {
    if (!is_flag_set(FLAG_V))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jv_reg(ExecutionContext* context) {
    if (!is_flag_set(FLAG_V))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnv_mem(ExecutionContext* context) {
    if (is_flag_set(FLAG_V))
        return EXECUTION_SUCCES;

    set_pc_bytes(
        context->operand_buffer[0],
        context->operand_buffer[1]
    );

    return EXECUTION_SUCCES;
}
static inline uint8_t op_jnv_reg(ExecutionContext* context) {
    if (is_flag_set(FLAG_V))
        return EXECUTION_SUCCES;

    uint8_t reg = context->operand_buffer[0];

    if (validate_reg(reg + 1))
        return EXECUTION_FAILED;

    set_pc_bytes(
        gRegisters[reg + 0],
        gRegisters[reg + 1]
    );

    return EXECUTION_SUCCES;
}

static inline uint8_t op_call_mem(ExecutionContext* context) {
    switch (context->step_cycle) {
        case 0:
            if (stack_push(context, get_pc() & 0xFF))
                return EXECUTION_FAILED;
            return EXECUTION_PENDING;

        case 1:
            if (stack_push(context, get_pc() >> 8))
                return EXECUTION_FAILED;
            return EXECUTION_PENDING;
            
        case 2:
            set_pc_bytes(
                context->operand_buffer[0], // Low
                context->operand_buffer[1]  // High
            );
            return EXECUTION_SUCCES;

        default:
            return EXECUTION_FAILED;
    }
}
static inline uint8_t op_call_reg(ExecutionContext* context) {
    switch (context->step_cycle) {
        case 0:
            if (stack_push(context, get_pc() & 0xFF))
                return EXECUTION_FAILED;
            return EXECUTION_PENDING;

        case 1:
            if (stack_push(context, get_pc() >> 8))
                return EXECUTION_FAILED;
            return EXECUTION_PENDING;
            
        case 2: {
            uint8_t reg = context->operand_buffer[0];

            if (validate_reg(reg + 1))
                return EXECUTION_FAILED;

            set_pc_bytes(
                gRegisters[0], // Low
                gRegisters[1]  // High
            );
            return EXECUTION_SUCCES;
        }

        default:
            return EXECUTION_FAILED;
    }
}
static inline uint8_t op_ret(ExecutionContext* context) {
    switch (context->step_cycle) {
        case 0:
            if(stack_pop(context, &gRegisters[6]))
                return EXECUTION_FAILED;
            return EXECUTION_PENDING;
        
        case 1: {
            uint8_t high;

            if (stack_pop(context, &high))
                return EXECUTION_FAILED;

            set_pc(gRegisters[6] | (high << 8));
            return EXECUTION_SUCCES;
        }

        default:
            return EXECUTION_FAILED;
    }
}
static inline uint8_t op_hlt(ExecutionContext* context) {
    gCpuHaltCode = HALTCODE_MANUAL;
    return EXECUTION_HALT;
}


