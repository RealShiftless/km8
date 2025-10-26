#include "cpu_internal.h"
#include "bus.h"

// Helpers


// Runtime functions
static void cpu_fetch_opcode(Km8Context* ctx) {
    uint16_t pc = km8_cpu_get_pc(&ctx->cpu);

    Km8BusAccess access = km8_bus_read(ctx, pc);
    ctx->cpu.instr_buf[0] = access.value;
    ctx->cpu.latency_cycles += access.latency_cycles;

    ctx->cpu.state = CPU_DECODE;

    ctx->cpu.opcode_pc = pc;

    km8_cpu_set_pc(&ctx->cpu, pc + 1);
}

static void cpu_decode(Km8Context* ctx) {
    ctx->cpu.selected_opcode = km8_cpu_get_opcode(ctx->cpu.instr_buf[0]);
    ctx->cpu.state = CPU_FETCH_OPERANDS;
}

static void cpu_fetch_operands(Km8Context* ctx) {
    
    uint16_t pc = km8_cpu_get_pc(&ctx->cpu);
    uint16_t pcDelta = pc - ctx->cpu.opcode_pc;

    if(pcDelta >= INSTR_BUF_SIZE) {
        km8_cpu_halt(ctx, CPU_HALTCODE_BUFFER_OVERFLOW);
        return;
    }

    if(pcDelta == ctx->cpu.selected_opcode->size) {
        ctx->cpu.state = CPU_EXECUTE;
        ctx->cpu.instr_exec_cycle = 0;
        return;
    }

    Km8BusAccess access = km8_bus_read(ctx, pc);
    ctx->cpu.instr_buf[pcDelta] = access.value;
    ctx->cpu.latency_cycles += access.latency_cycles;

    km8_cpu_set_pc(&ctx->cpu, pc + 1);
}

static void cpu_execute(Km8Context* ctx) {
    ExecutionResult result = ctx->cpu.selected_opcode->on_execute(ctx);

    switch(result) {
        case EXECUTION_PENDING:
            ctx->cpu.instr_exec_cycle++;
            return;
        
        case EXECUTION_SUCCESS:
            ctx->cpu.state = CPU_FETCH_OPCODE;
            ctx->cpu.instr_count++;
            return;

        case EXECUTION_FAILED:
            if(!ctx->cpu.halt_code) {
                ctx->cpu.halt_code = CPU_HALTCODE_EXEC_FAILED;
            }
            ctx->cpu.state = CPU_HALT;
            return;
    }
}

// Public func
void km8_cpu_step(Km8Context* ctx) {
    if(ctx->cpu.latency_cycles > 0) {
        ctx->cpu.latency_cycles--;
    }
    else {
        switch(ctx->cpu.state) {
            case CPU_FETCH_OPCODE:      cpu_fetch_opcode(ctx);      break;
            case CPU_DECODE:            cpu_decode(ctx);            break;
            case CPU_FETCH_OPERANDS:    cpu_fetch_operands(ctx);    break;
            case CPU_EXECUTE:           cpu_execute(ctx);           break;
            case CPU_HALT:              return;

            default:
                km8_cpu_halt(ctx, CPU_HALTCODE_INVALID_STATE);
                break;
        }
    }

    ctx->cpu.cycles++;
}

uint16_t km8_cpu_get_pc(const Cpu* cpu) {
    return cpu->registers[REG_PC + 0] | (cpu->registers[REG_PC + 1] << 8);
}
void km8_cpu_set_pc(Cpu* cpu, uint16_t addr) {
    uint8_t l_byte = addr & 0xFF;
    uint8_t h_byte = addr >> 8;

    cpu->registers[REG_PC + 0] = l_byte;
    cpu->registers[REG_PC + 1] = h_byte;
}

uint16_t km8_cpu_get_sp(const Cpu* cpu) {
    return cpu->registers[REG_SP + 0] | (cpu->registers[REG_SP + 1] << 8);
}
void km8_cpu_set_sp(Cpu* cpu, uint16_t addr) {
    uint8_t l_byte = addr & 0xFF;
    uint8_t h_byte = addr >> 8;

    cpu->registers[REG_SP + 0] = l_byte;
    cpu->registers[REG_SP + 1] = h_byte;
}


uint8_t km8_cpu_get_flags(const Cpu* cpu) {
    return cpu->registers[REG_FLAG];
}
void km8_cpu_set_flags(Cpu* cpu, CpuFlags flags) {
    cpu->registers[REG_FLAG] = (uint8_t) flags;
}


bool km8_cpu_is_flag_set(const Cpu* cpu, CpuFlags flag) {
    return km8_cpu_get_flags(cpu) & flag;
}
