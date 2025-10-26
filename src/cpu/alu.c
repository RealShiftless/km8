#include "cpu_internal.h"

uint8_t km8_cpu_alu_add(Cpu* cpu, uint8_t a, uint8_t b) {
    uint16_t wide_result = (uint16_t)a + (uint16_t)b;
    uint8_t result = (uint8_t)wide_result;

    CpuFlags flags = 0;

    if(wide_result > 0xFF)                                      flags |= FLAG_C;
    if(result == 0)                                             flags |= FLAG_Z;
    if(result & 0x80)                                           flags |= FLAG_N;
    if(((a ^ b) & 0x80) == 0 && ((a ^ result) & 0x80) != 0)     flags |= FLAG_V;

    km8_cpu_set_flags(cpu, flags);

    return result;
}

uint8_t km8_cpu_alu_adc(Cpu* cpu, uint8_t a, uint8_t b) {
    uint8_t carry_in = km8_cpu_is_flag_set(cpu, FLAG_C) ? 1u : 0u;
    uint16_t result = (uint16_t)a + (uint16_t)b + carry_in;
    uint8_t result8 = (uint8_t)result;
    uint8_t operand2 = (uint8_t)(b + carry_in);

    CpuFlags flags = 0;

    if(result > 0xFF)                                           flags |= FLAG_C;
    if(result8 == 0)                                            flags |= FLAG_Z;
    if(result8 & 0x80)                                          flags |= FLAG_N;
    if ((~(a ^ operand2) & (a ^ result8) & 0x80) != 0)          flags |= FLAG_V;

    km8_cpu_set_flags(cpu, flags);

    return result8;
}

uint8_t km8_cpu_alu_sub(Cpu* cpu, uint8_t a, uint8_t b) {
    uint16_t wide_result = (uint16_t)a - (uint16_t)b;
    uint8_t result = (uint8_t)wide_result;

    CpuFlags flags = 0;

    if(a < b)                                                 flags |= FLAG_C; // borrow occurred
    if(result == 0)                                           flags |= FLAG_Z;
    if(result & 0x80)                                         flags |= FLAG_N;
    if(((a ^ b) & 0x80) != 0 && ((a ^ result) & 0x80) != 0)   flags |= FLAG_V;

    km8_cpu_set_flags(cpu, flags);
    return result;
}

uint8_t km8_cpu_alu_sbc(Cpu* cpu, uint8_t a, uint8_t b) {
    uint8_t carry_in = km8_cpu_is_flag_set(cpu, FLAG_C) ? 1u : 0u;
    uint16_t wide_result = (uint16_t)a - ((uint16_t)b + carry_in);
    uint8_t result = (uint8_t)wide_result;
    uint8_t operand2 = (uint8_t)(b + carry_in);

    CpuFlags flags = 0;

    if(a < operand2)                                          flags |= FLAG_C; // borrow occurred
    if(result == 0)                                           flags |= FLAG_Z;
    if(result & 0x80)                                         flags |= FLAG_N;
    if(((a ^ operand2) & 0x80) != 0 && ((a ^ result) & 0x80) != 0) flags |= FLAG_V;

    km8_cpu_set_flags(cpu, flags);
    return result;
}

uint8_t km8_cpu_alu_and(Cpu* cpu, uint8_t a, uint8_t b) {
    uint8_t result = a & b;

    CpuFlags flags = 0;

    if(result == 0)               flags |= FLAG_Z;
    if(result & 0x80)             flags |= FLAG_N;

    km8_cpu_set_flags(cpu, flags);
    return result;
}

uint8_t km8_cpu_alu_or(Cpu* cpu, uint8_t a, uint8_t b) {
    uint8_t result = a | b;

    CpuFlags flags = 0;

    if(result == 0)               flags |= FLAG_Z;
    if(result & 0x80)             flags |= FLAG_N;

    km8_cpu_set_flags(cpu, flags);
    return result;
}

uint8_t km8_cpu_alu_xor(Cpu* cpu, uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;

    CpuFlags flags = 0;

    if(result == 0)               flags |= FLAG_Z;
    if(result & 0x80)             flags |= FLAG_N;

    km8_cpu_set_flags(cpu, flags);
    return result;
}
