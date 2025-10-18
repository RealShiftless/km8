#include "cpu.h"

#include "bus.h"

// Runtime functions
static void cpu_fetch_opcode(Cpu* cpu) {
    uint16_t pc = km8_cpu_get_pc(cpu);
}

static void cpu_decode(Cpu* cpu) {
}

static void cpu_fetch_operands(Cpu* cpu) {
    uint16_t pc = km8_cpu_get_pc(cpu);
}

static void cpu_execute(Cpu* cpu) {
}

// Public func
void km8_cpu_step(Cpu* cpu) {
    if(cpu->latency_cycles > 0) {
        cpu->latency_cycles--;
    }
    else {
        switch(cpu->state) {
            case CPU_FETCH_OPCODE:      cpu_fetch_opcode(cpu);      break;
            case CPU_DECODE:            cpu_decode(cpu);            break;
            case CPU_FETCH_OPERANDS:    cpu_fetch_operands(cpu);    break;
            case CPU_EXECUTE:           cpu_execute(cpu);           break;
            case CPU_HALT:              return;

            default:
                cpu->state = CPU_HALT;
                cpu->halt_code = CPU_HALTCODE_INVALID_STATE;
                break;
        }
    }

    cpu->cycles++;
}

uint16_t km8_cpu_get_pc(const Cpu* cpu) {
    return cpu->registers[REG_PC + 0] | (cpu->registers[REG_PC + 1] << 8);
}

uint16_t km8_cpu_get_sp(const Cpu* cpu) {
    return cpu->registers[REG_SP + 0] | (cpu->registers[REG_SP + 1] << 8);
}

uint8_t km8_cpu_get_flags(const Cpu* cpu) {
    return cpu->registers[REG_FLAG];
}

bool km8_cpu_is_flag_set(const Cpu* cpu, CpuFlags flag) {
    return km8_cpu_get_flags(cpu) & flag;
}