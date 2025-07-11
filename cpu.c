#include "cpu.h"

#include "rom.h"

Operator gOpcodes[MAX_OPCODES];

uint8_t gCpuState;

uint16_t gProgramCounter;
uint16_t gStackPointer;
uint8_t gRegisters[8];

static uint8_t gInstructionRegister;

static CpuExecutionContext gExecContext = { 0, 0, 0, 0, 0 };

void SetOpcode(uint8_t address, uint8_t size, char mnemonic[4], Operand operands[MAX_OPERANDS]) {
    gOpcodes[address].address = address;
    gOpcodes[address].size = size;
    memcpy(gOpcodes[address].mnemonic, mnemonic, MAX_MNEMONIC);

    if (operands != NULL) {
        memcpy(gOpcodes[address].operands, operands, sizeof(Operand) * MAX_OPERANDS);
    }
    else {
        memset(gOpcodes[address].operands, 0, sizeof(Operand) * MAX_OPERANDS);
    }
}

void CpuInit() {
    // Data
    SetOpcode(OPCODE_NOP,      1, "NOP",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    SetOpcode(OPCODE_LDR_MEM,  4, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    SetOpcode(OPCODE_LDR_IMM,  3, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_STR_MEM,  4, "STR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    SetOpcode(OPCODE_STR_IMM,  4, "STR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_MOV,      3, "MOV",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_SWAP,     3, "SWP",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_PUSH_REG, 2, "PUSH", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_PUSH_IMM, 2, "PUSH", (Operand[]) { { OPERAND_IMM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_POP,      2, "POP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_CLR,      2, "CLR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Addition
    SetOpcode(OPCODE_ADDU_REG, 3, "ADDU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_ADDU_IMM, 3, "ADDU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_ADDI_REG, 3, "ADDI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_ADDI_IMM, 3, "ADDI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_ADCU_REG, 3, "ADCU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_ADCU_IMM, 3, "ADCU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_ADCI_REG, 3, "ADCI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_ADCI_IMM, 3, "ADCI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_INC,      2, "INC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Subtraction
    SetOpcode(OPCODE_SUBU_REG, 3, "SUBU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_SUBU_IMM, 3, "SUBU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_SUBI_REG, 3, "SUBI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_SUBI_IMM, 3, "SUBI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_SUCU_REG, 3, "SUCU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_SUCU_IMM, 3, "SUCU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_SUCI_REG, 3, "SUCI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_SUCI_IMM, 3, "SUCI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_DEC,      2, "DEC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Multiplication
    SetOpcode(OPCODE_MULU_REG, 3, "MULU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_MULU_IMM, 3, "MULU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_MULI_REG, 3, "MULI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_MULI_IMM, 3, "MULI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Division
    SetOpcode(OPCODE_DIVU_REG, 3, "DIVU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_DIVU_IMM, 3, "DIVU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_DIVI_REG, 3, "DIVI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_DIVI_IMM, 3, "DIVI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Modulo
    SetOpcode(OPCODE_MODU_REG, 3, "MODU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_MODU_IMM, 3, "MODU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_MODI_REG, 3, "MODI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_MODI_IMM, 3, "MODI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Extra math
    SetOpcode(OPCODE_NEG,      2, "NEG",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Bitwise
    SetOpcode(OPCODE_AND_REG,  3, "AND",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_AND_IMM,  3, "AND",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_OR_REG,   3, "OR",   (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_OR_IMM,   3, "OR",   (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_XOR_REG,  3, "XOR",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_XOR_IMM,  3, "XOR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_NOT,      2, "NOT",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    SetOpcode(OPCODE_SHL,      2, "SHL",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_SHR,      2, "SHR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_ROL,      2, "ROL",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_ROR,      2, "ROR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });


    // Comparisons
    SetOpcode(OPCODE_CMPU_REG, 3, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_CMPU_IMM, 3, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    SetOpcode(OPCODE_CMPI_REG, 3, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_CMPI_IMM, 3, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    SetOpcode(OPCODE_TST_REG,  3, "TST",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    SetOpcode(OPCODE_TST_IMM,  3, "TST",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Conditionals
    SetOpcode(OPCODE_JMP_MEM,  3, "JMP",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JMP_REG,  2, "JMP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JZ_MEM,   3, "JZ",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JZ_REG,   2, "JZ",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNZ_MEM,  3, "JNZ",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNZ_REG,  2, "JNZ",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JC_MEM,   3, "JC",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JC_REG,   2, "JC",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNC_MEM,  3, "JNC",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNC_REG,  2, "JNC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JN_MEM,   3, "JN",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JN_REG,   2, "JN",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNN_MEM,  3, "JNN",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNN_REG,  2, "JNN",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JV_MEM,   3, "JV",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JV_REG,   2, "JV",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNV_MEM,  3, "JNV",  (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JNV_REG,  2, "JNV",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JL_REG,   3, "JL",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JL_MEM,   2, "JL",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JG_REG,   3, "JG",   (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_JG_MEM,   2, "JG",   (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_CALL_MEM, 3, "CALL", (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    SetOpcode(OPCODE_CALL_REG, 2, "CALL", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    SetOpcode(OPCODE_RET,      1, "RET",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    SetOpcode(OPCODE_HLT,      1, "HLT",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });

    // VBLANK :)
    SetOpcode(OPCODE_VBLK,     1, "VBLK", (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
}

void CpuStep() {
    switch (gCpuState) {
        case CPU_FETCH_OPCODE:
            gExecContext.cur_opcode = RomRead(gProgramCounter);
            gExecContext.opcode_pc = gProgramCounter;
            gProgramCounter++;

            gCpuState = CPU_DECODE;
            break;
        
        case CPU_DECODE:
            gExecContext.opcode_size = gOpcodes[gExecContext.cur_opcode].size;
            break;

        case CPU_FETCH_OPERANDS:

            break;

        case CPU_EXECUTE:

            break;

        case CPU_WRITEBACK:

            break;

        case CPU_HALT:
            return;
    }
}