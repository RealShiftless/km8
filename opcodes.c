#include "opcodes.h"


// Table
Operator gOpcodes[MAX_OPCODES];


// Opcode Func
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

void init_opcodes() {
    // Data
    set_opcode(OPCODE_NOP,      1, 4, "NOP",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    set_opcode(OPCODE_LDR_MEM,  4, 0, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    set_opcode(OPCODE_LDR_IMM,  3, 0, "LDR",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_STR,      4, 0, "STR",  (Operand[]) { { OPERAND_REG }, { OPERAND_MEM } });
    set_opcode(OPCODE_MOV,      3, 0, "MOV",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SWP,      3, 0, "SWP",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_PUSH,     2, 0, "PUSH", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_POP,      2, 0, "POP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_CLR,      2, 0, "CLR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Addition
    set_opcode(OPCODE_ADD_REG,  3, 0, "ADD",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADD_IMM,  3, 0, "ADD",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
                                
    set_opcode(OPCODE_ADC_REG,  3, 0, "ADC",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADC_IMM,  3, 0, "ADC",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_INC,      2, 0, "INC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Subtraction
    set_opcode(OPCODE_SUB_REG,  3, 0, "SUB",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUB_IMM,  3, 0, "SUB",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
                                              
    set_opcode(OPCODE_SBC_REG,  3, 0, "SBC",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SBC_IMM,  3, 0, "SBC",  (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
                                              
    set_opcode(OPCODE_DEC,      2, 0, "DEC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    set_opcode(OPCODE_CMP_REG,  3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_CMP_IMM,  3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

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
    set_opcode(OPCODE_CALL_MEM, 3, 0, "CALL", (Operand[]) { { OPERAND_MEM }, { OPERAND_NIL } });
    set_opcode(OPCODE_CALL_REG, 2, 0, "CALL", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_RET,      1, 0, "RET",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
    set_opcode(OPCODE_HLT,      1, 0, "HLT",  (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });

    // VBLANK :)
    //set_opcode(OPCODE_VBLK,     1, 0, "VBLK", (Operand[]) { { OPERAND_NIL }, { OPERAND_NIL } });
}