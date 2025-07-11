#include <stdint.h>
#include <time.h>
#include <stdio.h>

//#define CLOCK_SPEED 16780000
#define CLOCK_CYCLE_NS 5
#define MAX_CYCLE_DELTA CLOCK_CYCLE_NS * 5

#define MAX_OPCODES 256
#define MAX_MNEMONIC 4
#define MAX_OPERANDS 2

typedef enum {
	CTRL_READ  = 0b00000001,
	CTRL_WRITE = 0b00000010
};

typedef enum {
	CPU_FETCH_OPCODE,
	CPU_DECODE,        
	CPU_FETCH_OPERANDS, 
	CPU_EXECUTE, 
	CPU_WRITEBACK,
	CPU_HALT
} CpuState;

typedef enum {
	OPCODE_NOP = 0x00,
	OPCODE_LDR_MEM,
	OPCODE_LDR_IMM,
	OPCODE_STR_MEM,
	OPCODE_STR_IMM,
	OPCODE_MOV,
	OPCODE_SWAP,
	OPCODE_PUSH_REG,
	OPCODE_PUSH_IMM,
	OPCODE_POP,
	OPCODE_CLR,

	OPCODE_ADDU_REG = 0x10,
	OPCODE_ADDU_IMM,
	OPCODE_ADDI_REG,
	OPCODE_ADDI_IMM,

	OPCODE_ADCU_REG,
	OPCODE_ADCU_IMM,
	OPCODE_ADCI_REG,
	OPCODE_ADCI_IMM,

	OPCODE_INC,

	OPCODE_SUBU_REG,
	OPCODE_SUBU_IMM,
	OPCODE_SUBI_REG,
	OPCODE_SUBI_IMM,

	OPCODE_SUCU_REG,
	OPCODE_SUCU_IMM,
	OPCODE_SUCI_REG,
	OPCODE_SUCI_IMM,

	OPCODE_DEC,

	OPCODE_MULU_REG,
	OPCODE_MULU_IMM,
	OPCODE_MULI_REG,
	OPCODE_MULI_IMM,

	OPCODE_DIVU_REG,
	OPCODE_DIVU_IMM,
	OPCODE_DIVI_REG,
	OPCODE_DIVI_IMM,

	OPCODE_MODU_REG,
	OPCODE_MODU_IMM,
	OPCODE_MODI_REG,
	OPCODE_MODI_IMM,

	OPCODE_AND_REG,
	OPCODE_AND_IMM,
	OPCODE_OR_REG,
	OPCODE_OR_IMM,
	OPCODE_XOR_REG,
	OPCODE_XOR_IMM,
	OPCODE_NOT,
	OPCODE_NEG,
	OPCODE_SHL,
	OPCODE_SHR,
	OPCODE_ROL,
	OPCODE_ROR,

	OPCODE_CMPU_REG,
	OPCODE_CMPU_IMM,
	OPCODE_CMPI_REG,
	OPCODE_CMPI_IMM,
	OPCODE_TST_REG,
	OPCODE_TST_IMM,

	OPCODE_JMP_MEM = 0X40,
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
	OPCODE_JL_MEM,
	OPCODE_JL_REG,
	OPCODE_JG_MEM,
	OPCODE_JG_REG,
	
	OPCODE_CALL_MEM,
	OPCODE_CALL_REG,
	OPCODE_RET,
	OPCODE_HLT,

	OPCODE_VBLK = 0xFF

} Opcodes;

typedef enum {
	OPERAND_NIL = 0,
	OPERAND_IMM,
	OPERAND_REG,
	OPERAND_MEM
};

typedef struct {
	uint8_t cur_opcode;
	uint16_t opcode_pc;
	uint8_t opcode_size;

	uint8_t operandA_type;
	uint16_t operandA;

	uint8_t operandB_type;
	uint16_t operandB;

	uint16_t result;
} CpuExecutionContext;

typedef struct {
	uint8_t type;
} Operand;

typedef struct {
	uint8_t address;
	uint8_t size;

	char mnemonic[MAX_MNEMONIC];
	Operand operands[MAX_OPERANDS];
} Operator;

extern uint8_t gCpuState;

extern uint16_t gProgramCounter;
extern uint16_t gStackPointer;
extern uint8_t gRegisters[8];

extern uint16_t gAddressBus;
extern uint8_t gDataBus;

extern uint8_t gControlBus;


extern Operator gOpcodes[256];

void CpuInit(void);
void CpuStep(void);

static inline uint8_t IsFlagSet(uint8_t v, uint8_t f) {
	return (v & f) == f;
}
static inline uint8_t SetFlag(uint8_t v, uint8_t f) {
	return v | f;
}

#pragma once
