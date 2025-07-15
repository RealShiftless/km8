#include "cpu.h"

#include "emulator.h"
#include "bus.h"

Operator gOpcodes[MAX_OPCODES];

uint8_t gCpuCurrState = CPU_HALT;
uint8_t gCpuPrevState = CPU_HALT;

//uint16_t gProgramCounter;
//uint16_t gStackPointer;
uint8_t gRegisters[REGISTER_COUNT];

uint8_t gCpuHaltCode;

static uint8_t gInstructionRegister;
static uint8_t gLatencyCycles;

static uint16_t gDataBuffer;

static CpuExecutionContext gExecContext = { 0, 0, 0, 0, 0 };
static uint8_t gExecCycle;

// Sets the opcode at an adress with all the relevant info, we also store the address inside it for easy of use while debuggin, and
// Later on for assembling and such
void set_opcode(uint8_t address, uint8_t size, uint8_t cycles, char mnemonic[4], Operand operands[MAX_OPERANDS]) {
    gOpcodes[address].size = size;
    memcpy(gOpcodes[address].mnemonic, mnemonic, MAX_MNEMONIC);

    if (operands != NULL) {
        memcpy(gOpcodes[address].operands, operands, sizeof(Operand) * MAX_OPERANDS);
    }
    else {
        memset(gOpcodes[address].operands, 0, sizeof(Operand) * MAX_OPERANDS);
    }
}

// Here is prepare the cpu with different opcodes and their relevant info.
void cpu_init() {
    // Data
    set_opcode(OPCODE_NOP,      1, 4, "NOP",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    set_opcode(OPCODE_LDR_MEM,  4, 0, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    set_opcode(OPCODE_LDR_IMM,  3, 0, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_STR_REG,  4, 0, "STR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    set_opcode(OPCODE_STR_IMM,  4, 0, "STR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_MOV,      3, 0, "MOV",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SWP,     3, 0,  "SWP",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_PUSH_REG, 2, 0, "PUSH", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_PUSH_IMM, 2, 0, "PUSH", (Operand[]) { { OPERAND_IMM }, { OPERAND_NIL } });
    set_opcode(OPCODE_POP,      2, 0, "POP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_CLR,      2, 0, "CLR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Addition
    set_opcode(OPCODE_ADD_REG, 3, 0, "ADD",   (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADD_IMM, 3, 0, "ADD",   (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_ADC_REG, 3, 0, "ADC", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADC_IMM, 3, 0, "ADC", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_INC,      2, 0, "INC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Subtraction
    set_opcode(OPCODE_SUB_REG, 3, 0, "SUB", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUB_IMM, 3, 0, "SUB", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_SBC_REG, 3, 0, "SBC", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SBC_IMM, 3, 0, "SBC", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_DEC,      2, 0, "DEC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    set_opcode(OPCODE_CMP_REG, 3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_CMP_IMM, 3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_CMPS_REG, 3, 0, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_CMPS_IMM, 3, 0, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Bitwise
    set_opcode(OPCODE_AND_REG,  3, 0, "AND",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_AND_IMM,  3, 0, "AND",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_OR_REG,   3, 0, "OR",   (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_OR_IMM,   3, 0, "OR",   (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_XOR_REG,  3, 0, "XOR",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_XOR_IMM,  3, 0, "XOR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_NOT,      2, 0, "NOT",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    set_opcode(OPCODE_SHL,      2, 0, "SHL",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_SHR,      2, 0, "SHR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_ROL,      2, 0, "ROL",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_ROR,      2, 0, "ROR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    set_opcode(OPCODE_TST_REG,  3, 0, "TST",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_TST_IMM,  3, 0, "TST",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Conditionals
    set_opcode(OPCODE_JMP_MEM,  3, 0, "JMP",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JMP_REG,  2, 0, "JMP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JZ_MEM,   3, 0, "JZ",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JZ_REG,   2, 0, "JZ",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNZ_MEM,  3, 0, "JNZ",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNZ_REG,  2, 0, "JNZ",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JC_MEM,   3, 0, "JC",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JC_REG,   2, 0, "JC",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNC_MEM,  3, 0, "JNC",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNC_REG,  2, 0, "JNC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JN_MEM,   3, 0, "JN",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JN_REG,   2, 0, "JN",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNN_MEM,  3, 0, "JNN",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNN_REG,  2, 0, "JNN",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JV_MEM,   3, 0, "JV",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JV_REG,   2, 0, "JV",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNV_MEM,  3, 0, "JNV",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JNV_REG,  2, 0, "JNV",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JL_REG,   3, 0, "JL",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JL_MEM,   2, 0, "JL",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_JG_REG,   3, 0, "JG",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_JG_MEM,   2, 0, "JG",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_CALL_MEM, 3, 0, "CALL", (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_CALL_REG, 2, 0, "CALL", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_RET,      1, 0, "RET",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    set_opcode(OPCODE_HLT,      1, 0, "HLT",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });

    // VBLANK :)
    set_opcode(OPCODE_VBLK,     1, 0, "VBLK", (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
}

// Math Operations, easier to do here for different versions and flags and stuff
static inline uint8_t op_add(uint8_t a, uint8_t b) {
    uint16_t result = a + b;

    gFlags = 0;

    // Carry if result overflowed 8 bits
    if (result > 0xFF) gFlags |= FLAG_C;

    // Zero if result is 0
    if ((uint8_t)result == 0) gFlags |= FLAG_Z;

    // Negative if bit 7 set
    if (result & 0x80) gFlags |= FLAG_N;

    // Overflow if signs of inputs are same and sign of result differs
    if (((a ^ b) & 0x80) == 0 && ((a ^ result) & 0x80) != 0) gFlags |= FLAG_V;

    return (uint8_t)result;
}

static inline uint8_t op_adc(uint8_t a, uint8_t b) {
    // Get carry flag if set
    uint8_t carry_in = (gFlags & FLAG_C) ? 1 : 0;

    gFlags = 0;

    // Full 9-bit result for overflow/carry checking
    uint16_t result = a + b + carry_in;

    // Clear old flags before setting new ones
    gFlags = 0;

    // Carry out of bit 7 (unsigned overflow)
    if (result > 0xFF) gFlags |= FLAG_C;

    // Zero result
    if ((uint8_t)result == 0) gFlags |= FLAG_Z;

    // Negative flag (sign bit)
    if (result & 0x80) gFlags |= FLAG_N;

    // Overflow (signed overflow detection)
    if (((a ^ b) & 0x80) == 0 && ((a ^ result) & 0x80) != 0) gFlags |= FLAG_V;

    return (uint8_t)result;
}

static inline uint8_t op_sub(uint8_t a, uint8_t b) {
    uint16_t result = a - b;

    gFlags = 0;

    // Carry if borrow
    if (a < b) gFlags |= FLAG_C;

    // Zero if 0
    if ((uint8_t)result == 0) gFlags |= FLAG_Z;

    // Negative if bit 7
    if (result & 0x80) gFlags |= FLAG_N;

    // Overflow if signs of inputs are same and sign of result differs
    if (((a ^ b) & 0x80) != 0 && ((a ^ result) & 0x80) != 0) gFlags |= FLAG_V;

    return (uint8_t)result;
}

static inline uint8_t op_sbc(uint8_t a, uint8_t b) {
    uint8_t borrow = (gFlags & FLAG_C) ? 1 : 0;
    uint16_t result = a - b - borrow;

    gFlags = 0;

    // Set Carry if borrow occurred (unsigned underflow)
    if (a < (uint16_t)b + borrow) gFlags |= FLAG_C;

    // Set Zero if result is zero
    if ((uint8_t)result == 0) gFlags |= FLAG_Z;

    // Set Negative if sign bit is set
    if (result & 0x80) gFlags |= FLAG_N;

    // Set Overflow if signs differ and result sign is unexpected
    if (((a ^ b) & 0x80) != 0 && ((a ^ result) & 0x80) != 0) gFlags |= FLAG_V;

    return (uint8_t)result;
}

static inline uint8_t op_and(uint8_t a, uint8_t b) {
    uint8_t result = a & b;

    gFlags = 0;

    if (result == 0) gFlags |= FLAG_Z;

    return result;
}

static inline uint8_t op_or(uint8_t a, uint8_t b) {
    uint8_t result = a | b;

    gFlags = 0;

    if (result == 0) gFlags |= FLAG_Z;

    return result;
}

static inline uint8_t op_xor(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;

    gFlags = 0;

    if (result == 0) gFlags |= FLAG_Z;

    return result;
}

// Cpu Execution Func
static inline uint8_t execute_opcode(uint8_t cycle) {
    BusAccessResult busResult = BUS_OK;
    uint8_t opcodeResult = EXECUTION_SUCCES;

    switch (gExecContext.cur_opcode) {
    case OPCODE_NOP:
        break;

    case OPCODE_LDR_MEM:
        if (cycle == 0) {
            uint16_t addr = gExecContext.operand_buffer[1] | (gExecContext.operand_buffer[2] << 8);
            busResult = bus_read(addr, &gDataBuffer, &gLatencyCycles);
            opcodeResult = EXECUTION_PENDING;
        }
        else if (cycle == 1) {
            gRegisters[gExecContext.operand_buffer[0]] = gDataBuffer;
        }
        break;

    case OPCODE_LDR_IMM:
        gRegisters[gExecContext.operand_buffer[0]] = gExecContext.operand_buffer[1];
        break;

    case OPCODE_STR_REG: {
        uint16_t addr = gExecContext.operand_buffer[1] | (gExecContext.operand_buffer[2] << 8);
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT)
        {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        busResult = bus_write(addr, gRegisters[reg], &gLatencyCycles);
        break;
    }

    case OPCODE_MOV: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = gRegisters[reg1];
        break;
    }

    case OPCODE_SWP: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        uint8_t temp = gRegisters[reg0];
        gRegisters[reg0] = gRegisters[reg1];
        gRegisters[reg1] = temp;
        break;
    }

    case OPCODE_PUSH_REG: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT)
        {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        busResult = bus_write(gStackPointer--, gRegisters[reg], &gLatencyCycles);
        break;
    }

    case OPCODE_PUSH_IMM: {
        busResult = bus_write(gStackPointer--, gExecContext.operand_buffer[0], &gLatencyCycles);
        break;
    }

    case OPCODE_POP: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT)
        {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        busResult = bus_read(gStackPointer++, &gRegisters[reg], &gLatencyCycles);
        break;
    }

    case OPCODE_CLR: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT)
        {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = 0;
        break;
    }

    case OPCODE_ADD_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        // actually set the result
        gRegisters[reg0] = op_add(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_ADD_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_add(gRegisters[reg], imm);
        break;
    }

    case OPCODE_ADC_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_adc(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_ADC_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_adc(gRegisters[reg], imm);
        break;
    }

    case OPCODE_INC: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_add(gRegisters[reg], 1);
        break;
    }

    case OPCODE_SUB_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_sub(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_SUB_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_sub(gRegisters[reg], imm);
        break;
    }

    case OPCODE_SBC_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_sbc(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_SBC_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_sbc(gRegisters[reg], imm);
        break;
    }

    case OPCODE_DEC: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_sub(gRegisters[reg], 1);
        break;
    }

    case OPCODE_CMP_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        op_sub(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_CMP_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        op_sub(gRegisters[reg], imm);
        break;
    }

    case OPCODE_AND_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_and(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_AND_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_and(gRegisters[reg], imm);
        break;
    }

    case OPCODE_OR_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_or(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_OR_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_or(gRegisters[reg], imm);
        break;
    }

    case OPCODE_XOR_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg0] = op_xor(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_XOR_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = op_xor(gRegisters[reg], imm);
        break;
    }

    case OPCODE_NOT: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = !gRegisters[reg];

        if (gRegisters[reg] == 0) gFlags |= FLAG_Z;
        break;
    }

    case OPCODE_SHL: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = gRegisters[reg] << 1;

        if (gRegisters[reg] == 0) gFlags |= FLAG_Z;
        break;
    }

    case OPCODE_SHR: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = gRegisters[reg] >> 1;

        if (gRegisters[reg] == 0) gFlags |= FLAG_Z;
        break;
    }

    case OPCODE_ROL: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = (gRegisters[reg] << 1) | (gRegisters[reg] >> 7);

        if (gRegisters[reg] == 0) gFlags |= FLAG_Z;
        break;
    }

    case OPCODE_ROR: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gRegisters[reg] = (gRegisters[reg] >> 1) | (gRegisters[reg] << 7);

        if (gRegisters[reg] == 0) gFlags |= FLAG_Z;
        break;
    }

    case OPCODE_TST_REG: {
        uint8_t reg0 = gExecContext.operand_buffer[0];
        uint8_t reg1 = gExecContext.operand_buffer[1];

        if (reg0 >= REGISTER_COUNT || reg1 >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        op_and(gRegisters[reg0], gRegisters[reg1]);
        break;
    }

    case OPCODE_TST_IMM: {
        uint8_t reg = gExecContext.operand_buffer[0];
        uint8_t imm = gExecContext.operand_buffer[1];

        if (reg >= REGISTER_COUNT) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        op_and(gRegisters[reg], imm);
        break;
    }
    
    case OPCODE_JMP_MEM: {
        gProgramCounter = *(uint16_t*)&gExecContext.operand_buffer[0];

        uint16_t val = gProgramCounter;
        break;
    }

    case OPCODE_JMP_REG: {
        uint8_t reg = gExecContext.operand_buffer[0];

        if (reg >= REGISTER_COUNT - 1) {
            gCpuHaltCode = HALTCODE_INVALID_OPERAND;
            opcodeResult = EXECUTION_FAILED;
            break;
        }

        gProgramCounter = *(uint16_t*)&gRegisters[reg];
        break;
    }

    case OPCODE_HLT:
        gCpuHaltCode = HALTCODE_MANUAL;
        return EXECUTION_HALT;
    
    default:
        gCpuHaltCode = HALTCODE_INVALID_OPCODE;
        return EXECUTION_FAILED;
    }

    // Centralized error handling
    if (busResult != BUS_OK) {
        gCpuHaltCode =
            (busResult == BUS_HALT_READONLY) ? HALTCODE_WRITE_PROTECTED :
            (busResult == BUS_HALT_UNUSED) ? HALTCODE_BUS_FAULT :
            (busResult == BUS_HALT_INVALID) ? HALTCODE_BUS_FAULT :
            HALTCODE_UNKNOWN;

        gCpuCurrState = CPU_HALT;

        return EXECUTION_FAILED;
    }

    return opcodeResult;
}

// Steps the cpu and sets relevant state
void cpu_run_cycle() {
    uint16_t pcDelta = gProgramCounter - gExecContext.opcode_pc;

    if (gLatencyCycles > 0) {
        gLatencyCycles--;
        return;
    }

    switch (gCpuCurrState) {
        case CPU_FETCH_OPCODE:
            gExecContext.opcode_pc = gProgramCounter;
            if (bus_read(gProgramCounter++, &gExecContext.cur_opcode, &gLatencyCycles) != BUS_OK)
                return;

            gCpuCurrState = CPU_DECODE;
            break;

        case CPU_DECODE:
            if (gOpcodes[gExecContext.cur_opcode].size == 0)
            {
                gCpuCurrState = CPU_HALT;
                gCpuHaltCode = HALTCODE_INVALID_OPCODE;
                return;
            }

            gExecContext.opcode_size = gOpcodes[gExecContext.cur_opcode].size;
            gExecContext.operandA_type = gOpcodes[gExecContext.cur_opcode].operands[0].type;
            gExecContext.operandB_type = gOpcodes[gExecContext.cur_opcode].operands[1].type;

            gCpuCurrState = CPU_FETCH_OPERANDS;
            break;

        case CPU_FETCH_OPERANDS:
            if (pcDelta == gExecContext.opcode_size)
            {
                gCpuCurrState |= CPU_EXECUTE;
                gExecCycle = 0;
                return;
            }

            if (bus_read(gProgramCounter++, &gExecContext.operand_buffer[pcDelta - 1], &gLatencyCycles) != BUS_OK)
                return;
            break;

        case CPU_EXECUTE:
            switch (execute_opcode(gExecCycle++))
            {
                case EXECUTION_HALT:
                case EXECUTION_FAILED:
                    emu_halt();
                    return;

                case EXECUTION_SUCCES:
                    gCpuCurrState = CPU_FETCH_OPCODE;
                    break;
            }
            break;


        case CPU_WRITEBACK:

            break;

        case CPU_HALT:
            return;

        default:
            gCpuCurrState = CPU_HALT;
            gCpuHaltCode = HALTCODE_UNKNOWN;
            break;
    }

    gCurCycle++;
}