#include "cpu.h"

#include "emulator.h"
#include "bus.h"

Operator gOpcodes[MAX_OPCODES];

uint8_t gCpuCurrState = CPU_HALT;
uint8_t gCpuPrevState = CPU_HALT;

uint16_t gProgramCounter;
uint16_t gStackPointer;
uint8_t gRegisters[8];

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
    set_opcode(OPCODE_SWAP,     3, 0, "SWP",  (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_PUSH_REG, 2, 0, "PUSH", (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_PUSH_IMM, 2, 0, "PUSH", (Operand[]) { { OPERAND_IMM }, { OPERAND_NIL } });
    set_opcode(OPCODE_POP,      2, 0, "POP",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });
    set_opcode(OPCODE_CLR,      2, 0, "CLR",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Addition
    set_opcode(OPCODE_ADDU_REG, 3, 0, "ADDU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADDU_IMM, 3, 0, "ADDU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_ADDI_REG, 3, 0, "ADDI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADDI_IMM, 3, 0, "ADDI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_ADCU_REG, 3, 0, "ADCU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADCU_IMM, 3, 0, "ADCU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_ADCI_REG, 3, 0, "ADCI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_ADCI_IMM, 3, 0, "ADCI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_INC,      2, 0, "INC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Subtraction
    set_opcode(OPCODE_SUBU_REG, 3, 0, "SUBU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUBU_IMM, 3, 0, "SUBU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_SUBI_REG, 3, 0, "SUBI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUBI_IMM, 3, 0, "SUBI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_SUCU_REG, 3, 0, "SUCU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUCU_IMM, 3, 0, "SUCU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_SUCI_REG, 3, 0, "SUCI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_SUCI_IMM, 3, 0, "SUCI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    set_opcode(OPCODE_DEC,      2, 0, "DEC",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

    // Multiplication
    set_opcode(OPCODE_MULU_REG, 3, 0, "MULU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_MULU_IMM, 3, 0, "MULU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_MULI_REG, 3, 0, "MULI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_MULI_IMM, 3, 0, "MULI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Division
    set_opcode(OPCODE_DIVU_REG, 3, 0, "DIVU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_DIVU_IMM, 3, 0, "DIVU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_DIVI_REG, 3, 0, "DIVI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_DIVI_IMM, 3, 0, "DIVI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Modulo
    set_opcode(OPCODE_MODU_REG, 3, 0, "MODU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_MODU_IMM, 3, 0, "MODU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_MODI_REG, 3, 0, "MODI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_MODI_IMM, 3, 0, "MODI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

    // Extra math
    set_opcode(OPCODE_NEG,      2, 0, "NEG",  (Operand[]) { { OPERAND_REG }, { OPERAND_NIL } });

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


    // Comparisons
    set_opcode(OPCODE_CMPU_REG, 3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_CMPU_IMM, 3, 0, "CMPU", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });
    set_opcode(OPCODE_CMPI_REG, 3, 0, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_REG } });
    set_opcode(OPCODE_CMPI_IMM, 3, 0, "CMPI", (Operand[]) { { OPERAND_REG }, { OPERAND_IMM } });

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

// Cpu Execution Func
static inline uint8_t execute_opcode(uint8_t cycle) {
    BusAccessResult busResult = BUS_OK;
    uint8_t opcodeResult = EXECUTION_FAILED;

    switch (gExecContext.cur_opcode) {
    case OPCODE_NOP:
        opcodeResult = EXECUTION_SUCCES;
        break;

    case OPCODE_LDR_MEM:
        if (cycle == 0) {
            uint16_t addr = gExecContext.operand_buffer[1] | (gExecContext.operand_buffer[2] << 8);
            busResult = bus_read(addr, &gDataBuffer, &gLatencyCycles);
            opcodeResult = EXECUTION_PENDING;
        }
        else if (cycle == 1) {
            gRegisters[gExecContext.operand_buffer[0]] = gDataBuffer;
            opcodeResult = EXECUTION_SUCCES;
        }
        break;

    case OPCODE_LDR_IMM:
        gRegisters[gExecContext.operand_buffer[0]] = gExecContext.operand_buffer[1];
        opcodeResult = EXECUTION_SUCCES;
        break;

    case OPCODE_STR_REG: {
        uint16_t addr = gExecContext.operand_buffer[1] | (gExecContext.operand_buffer[2] << 8);
        uint8_t value = gRegisters[gExecContext.operand_buffer[0]];
        busResult = bus_write(addr, value, &gLatencyCycles);
        opcodeResult = EXECUTION_SUCCES;
        break;
    }

    case OPCODE_HLT:
        gCpuHaltCode = HALTCODE_MANUAL;
        return EXECUTION_HALT;

    case OPCODE_JMP_MEM:
        gProgramCounter = gExecContext.operand_buffer[1] | (gExecContext.operand_buffer[0] << 8);
        opcodeResult = EXECUTION_SUCCES;
        break;
    
    default:
        gCpuHaltCode = HALTCODE_INVALID_OPCODE;
        return EXECUTION_FAILED;
    }

    // Centralized error handling
    if (busResult != BUS_OK) {
        gCpuHaltCode =
            (busResult == BUS_HALT_READONLY) ? HALTCODE_READ_PROTECTED :
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