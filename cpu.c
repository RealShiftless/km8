#include "cpu.h"

#include "emulator.h"
#include "opcodes.h"


uint8_t gCpuState = CPU_HALT;
uint8_t gCpuPrevState = CPU_HALT;
uint8_t gCpuHaltCode;

uint8_t gRegisters[REGISTER_COUNT];

static ExecutionContext gExecContext = { 0 };
static uint8_t gExecCycle;


// Here is prepare the cpu with different opcodes and their relevant info.
void init_cpu() {
    memset(gRegisters, 0, REGISTER_COUNT);

    gCpuState = CPU_FETCH_OPCODE;
    gCpuHaltCode = HALTCODE_NONE;
}


// Cpu Execution Func
static inline uint8_t execute_opcode(ExecutionContext* context) {
    BusAccessResult busResult = BUS_OK;
    uint8_t opcodeResult = EXECUTION_SUCCES;

    uint16_t sp = get_sp();
    uint16_t pc = get_pc();

    switch (context->opcode) {
        // Memory
        case OPCODE_NOP:        return op_nop(context);
        case OPCODE_LDR_MEM:    return op_ldr_mem(context);
        case OPCODE_LDR_IMM:    return op_ldr_imm(context);
        case OPCODE_STR:        return op_str(context);
        case OPCODE_MOV:        return op_mov(context);
        case OPCODE_SWP:        return op_swp(context);
        case OPCODE_PUSH:       return op_push(context);
        case OPCODE_POP:        return op_pop(context);
        case OPCODE_CLR:        return op_clr(context);

        // Arithmetic
        case OPCODE_ADD_REG:    return op_add_reg(context);
        case OPCODE_ADD_IMM:    return op_add_imm(context);
        case OPCODE_ADC_REG:    return op_adc_reg(context);
        case OPCODE_ADC_IMM:    return op_adc_imm(context);
        case OPCODE_INC:        return op_inc(context);
        case OPCODE_SUB_REG:    return op_sub_reg(context);
        case OPCODE_SUB_IMM:    return op_sub_imm(context);
        case OPCODE_SBC_REG:    return op_sbc_reg(context);
        case OPCODE_SBC_IMM:    return op_sbc_imm(context);
        case OPCODE_DEC:        return op_dec(context);
        case OPCODE_CMP_REG:    return op_cmp_reg(context);
        case OPCODE_CMP_IMM:    return op_cmp_imm(context);

        // Bitwise
        case OPCODE_AND_REG:    return op_and_reg(context);
        case OPCODE_AND_IMM:    return op_and_imm(context);
        case OPCODE_OR_REG:     return op_or_reg(context);
        case OPCODE_OR_IMM:     return op_or_imm(context);
        case OPCODE_XOR_REG:    return op_xor_reg(context);
        case OPCODE_XOR_IMM:    return op_xor_imm(context);
        case OPCODE_NOT:        return op_not(context);
        case OPCODE_SHL:        return op_shl(context);
        case OPCODE_SHR:        return op_shr(context);
        case OPCODE_ROL:        return op_rol(context);
        case OPCODE_ROR:        return op_ror(context);
        case OPCODE_TST_REG:    return op_tst_reg(context);
        case OPCODE_TST_IMM:    return op_tst_imm(context);

        // Control Flow
        case OPCODE_JMP_MEM:    return op_jmp_mem(context);
        case OPCODE_JMP_REG:    return op_jmp_reg(context);
        case OPCODE_JZ_MEM:     return op_jz_mem(context);
        case OPCODE_JZ_REG:     return op_jz_reg(context);
        case OPCODE_JNZ_MEM:    return op_jnz_mem(context);
        case OPCODE_JNZ_REG:    return op_jnz_reg(context);
        case OPCODE_JC_MEM:     return op_jc_mem(context);
        case OPCODE_JC_REG:     return op_jc_reg(context);
        case OPCODE_JNC_MEM:    return op_jnc_mem(context);
        case OPCODE_JNC_REG:    return op_jnc_reg(context);
        case OPCODE_JN_MEM:     return op_jn_mem(context);
        case OPCODE_JN_REG:     return op_jn_reg(context);
        case OPCODE_JNN_MEM:    return op_jnn_mem(context);
        case OPCODE_JNN_REG:    return op_jnn_reg(context);
        case OPCODE_JV_MEM:     return op_jv_mem(context);
        case OPCODE_JV_REG:     return op_jv_reg(context);
        case OPCODE_JNV_MEM:    return op_jnv_mem(context);
        case OPCODE_JNV_REG:    return op_jnv_reg(context);
        case OPCODE_CALL_MEM:   return op_call_mem(context);
        case OPCODE_CALL_REG:   return op_call_reg(context);
        case OPCODE_RET:        return op_ret(context);
        case OPCODE_HLT:        return op_hlt(context);

        default:
            gCpuHaltCode = HALTCODE_INVALID_OPCODE;
            return EXECUTION_FAILED;
    }
}

// CPU States
static inline void cpu_fetch_opcode(ExecutionContext* context) {
    uint16_t pc = get_pc();

    context->opcode_pc = pc;

    if (handle_bus_read(pc, &context->opcode, &context->latency_cycles))
        return;

    set_pc(pc + 1);
    gCpuState = CPU_DECODE;
}

static inline void cpu_decode(ExecutionContext* context) {
    if (gOpcodes[gExecContext.opcode].size == 0)
    {
        gCpuState = CPU_HALT;
        gCpuHaltCode = HALTCODE_INVALID_OPCODE;
        return;
    }

    gExecContext.opcode_size = gOpcodes[gExecContext.opcode].size;
    gExecContext.operandA_type = gOpcodes[gExecContext.opcode].operands[0].type;
    gExecContext.operandB_type = gOpcodes[gExecContext.opcode].operands[1].type;

    gCpuState = CPU_FETCH_OPERANDS;
}

static inline void cpu_fetch_operands(ExecutionContext* context) {
    uint16_t pc = get_pc();
    uint16_t pcDelta = pc - context->opcode_pc;

    if (pcDelta == gExecContext.opcode_size)
    {
        gCpuState |= CPU_EXECUTE;
        return;
    }

    if (handle_bus_read(pc, &context->operand_buffer[pcDelta - 1], &context->latency_cycles) != BUS_OK)
        return;

    set_pc(pc + 1);
}

static inline void cpu_execute(ExecutionContext* context) {
    switch (execute_opcode(context))
    {
        case EXECUTION_HALT:
        case EXECUTION_FAILED:
            emu_halt();
            return;

        case EXECUTION_SUCCES:
            gCpuState = CPU_FETCH_OPCODE;
            break;
    }
}

// Steps the cpu and sets relevant state
void cpu_run_cycle() {
    if (gExecContext.latency_cycles > 0) {
        gExecContext.latency_cycles--;
        return;
    }

    uint16_t curPc = get_pc();

    switch (gCpuState) {
        case CPU_FETCH_OPCODE:   cpu_fetch_opcode(&gExecContext);   break;
        case CPU_DECODE:         cpu_decode(&gExecContext);         break;
        case CPU_FETCH_OPERANDS: cpu_fetch_operands(&gExecContext); break;
        case CPU_EXECUTE:        cpu_execute(&gExecContext);        break;
        case CPU_HALT:           emu_halt();                        return;

        default:
            gCpuState = CPU_HALT;
            gCpuHaltCode = HALTCODE_UNKNOWN;
            break;
    }

    gCurCycle++;
}



