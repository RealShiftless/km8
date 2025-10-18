#include "cpu_internal.h"

#include "km8/km8.h"
#include "bus.h"

#define KM8_VALIDATE_REGISTER(reg) \
    do {                           \
        if ((reg) >= REGISTER_COUNT) \
            return EXECUTION_FAILED; \
    } while (0)

#define KM8_VALIDATE_REGISTER_PAIR(reg) \
    do {                                 \
        KM8_VALIDATE_REGISTER(reg);       \
        KM8_VALIDATE_REGISTER((reg) + 1); \
    } while (0)

#define OPCODE(func, size) ((Km8Opcode){ (func), (size) })

// Helpers
static inline uint16_t get_addr(const uint8_t* value) {
    return (uint16_t)value[0] | ((uint16_t)value[1] << 8);
}

static inline uint8_t bus_read(Km8Context* ctx, uint16_t addr) {
    Km8BusAccess result = km8_bus_read(ctx, addr);
    ctx->cpu.latency_cycles += result.latency_cycles;
    return result.value;
}

static inline void bus_write(Km8Context* ctx, uint16_t addr, uint8_t value) {
    Km8BusAccess result = km8_bus_write(ctx, addr, value);
    ctx->cpu.latency_cycles += result.latency_cycles;
}

static inline void stack_push(Km8Context* ctx, uint8_t value) {
    uint16_t sp = km8_cpu_get_sp(&ctx->cpu);
    bus_write(ctx, sp, value);
    km8_cpu_set_sp(&ctx->cpu, sp - 1);
}

static inline uint8_t stack_pop(Km8Context* ctx) {
    uint16_t sp = km8_cpu_get_sp(&ctx->cpu);
    uint8_t value = bus_read(ctx, sp + 1);
    km8_cpu_set_sp(&ctx->cpu, sp + 1);
    return value;
}

// Opcode implementations
static ExecutionResult op_nop(Km8Context* ctx) {
    return EXECUTION_SUCCESS;
}

// Args: reg, mem_low, mem_high
static ExecutionResult op_ldr_mem(Km8Context* ctx) {
    switch (ctx->cpu.instr_exec_cycle) {
        case 0: {
            uint16_t addr = get_addr(ctx->cpu.instr_buf + 2);
            ctx->cpu.value_buf = bus_read(ctx, addr);
            return EXECUTION_PENDING;
        }
        case 1: {
            uint8_t reg = ctx->cpu.instr_buf[1];
            KM8_VALIDATE_REGISTER(reg);
            ctx->cpu.registers[reg] = ctx->cpu.value_buf;
            return EXECUTION_SUCCESS;
        }
        default:
            return EXECUTION_FAILED;
    }
}

// Args: reg, imm
static ExecutionResult op_ldr_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    ctx->cpu.value_buf = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = ctx->cpu.value_buf;
    return EXECUTION_SUCCESS;
}

// Args: reg, mem_low, mem_high
static ExecutionResult op_str(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint16_t addr = get_addr(ctx->cpu.instr_buf + 2);
    bus_write(ctx, addr, value);
    return EXECUTION_SUCCESS;
}

// Args: reg1, reg2
static ExecutionResult op_mov(Km8Context* ctx) {
    uint8_t reg_dst = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg_dst);
    uint8_t reg_src = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg_src);

    ctx->cpu.registers[reg_dst] = ctx->cpu.registers[reg_src];
    return EXECUTION_SUCCESS;
}

// Args: reg1, reg2
static ExecutionResult op_swp(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    uint8_t tmp = ctx->cpu.registers[reg0];
    ctx->cpu.registers[reg0] = ctx->cpu.registers[reg1];
    ctx->cpu.registers[reg1] = tmp;
    return EXECUTION_SUCCESS;
}

// Args: reg
static ExecutionResult op_push(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    stack_push(ctx, ctx->cpu.registers[reg]);
    return EXECUTION_SUCCESS;
}

// Args: reg
static ExecutionResult op_pop(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    ctx->cpu.registers[reg] = stack_pop(ctx);
    return EXECUTION_SUCCESS;
}

// Args: reg
static ExecutionResult op_clr(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    ctx->cpu.registers[reg] = 0;
    km8_cpu_set_flags(&ctx->cpu, FLAG_Z);
    return EXECUTION_SUCCESS;
}

// --- Arithmetic / logic --------------------------------------------------

// Args: reg_dst, reg_src
static ExecutionResult op_add_reg(Km8Context* ctx) {
    uint8_t reg_dst = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg_dst);
    uint8_t reg_src = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg_src);

    ctx->cpu.registers[reg_dst] = km8_cpu_alu_add(
        &ctx->cpu,
        ctx->cpu.registers[reg_dst],
        ctx->cpu.registers[reg_src]);
    return EXECUTION_SUCCESS;
}

// Args: reg, imm
static ExecutionResult op_add_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_add(
        &ctx->cpu,
        ctx->cpu.registers[reg],
        imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_adc_reg(Km8Context* ctx) {
    uint8_t reg_dst = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg_dst);
    uint8_t reg_src = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg_src);

    ctx->cpu.registers[reg_dst] = km8_cpu_alu_adc(
        &ctx->cpu,
        ctx->cpu.registers[reg_dst],
        ctx->cpu.registers[reg_src]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_adc_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_adc(
        &ctx->cpu,
        ctx->cpu.registers[reg],
        imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_inc(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    ctx->cpu.registers[reg] = km8_cpu_alu_add(&ctx->cpu, ctx->cpu.registers[reg], 1u);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_sub_reg(Km8Context* ctx) {
    uint8_t reg_dst = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg_dst);
    uint8_t reg_src = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg_src);

    ctx->cpu.registers[reg_dst] = km8_cpu_alu_sub(
        &ctx->cpu,
        ctx->cpu.registers[reg_dst],
        ctx->cpu.registers[reg_src]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_sub_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_sub(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_sbc_reg(Km8Context* ctx) {
    uint8_t reg_dst = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg_dst);
    uint8_t reg_src = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg_src);

    ctx->cpu.registers[reg_dst] = km8_cpu_alu_sbc(
        &ctx->cpu,
        ctx->cpu.registers[reg_dst],
        ctx->cpu.registers[reg_src]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_sbc_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_sbc(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_dec(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    ctx->cpu.registers[reg] = km8_cpu_alu_sub(&ctx->cpu, ctx->cpu.registers[reg], 1u);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_cmp_reg(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    (void)km8_cpu_alu_sub(&ctx->cpu, ctx->cpu.registers[reg0], ctx->cpu.registers[reg1]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_cmp_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    (void)km8_cpu_alu_sub(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_and_reg(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    ctx->cpu.registers[reg0] = km8_cpu_alu_and(&ctx->cpu, ctx->cpu.registers[reg0], ctx->cpu.registers[reg1]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_and_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_and(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_or_reg(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    ctx->cpu.registers[reg0] = km8_cpu_alu_or(&ctx->cpu, ctx->cpu.registers[reg0], ctx->cpu.registers[reg1]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_or_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_or(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_xor_reg(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    ctx->cpu.registers[reg0] = km8_cpu_alu_xor(&ctx->cpu, ctx->cpu.registers[reg0], ctx->cpu.registers[reg1]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_xor_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    ctx->cpu.registers[reg] = km8_cpu_alu_xor(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_not(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t result = (uint8_t)~ctx->cpu.registers[reg];
    ctx->cpu.registers[reg] = result;

    CpuFlags flags = 0;
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80) flags |= FLAG_N;
    km8_cpu_set_flags(&ctx->cpu, flags);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_shl(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint8_t carry = (value & 0x80u) ? 1u : 0u;
    uint8_t result = (uint8_t)(value << 1);
    ctx->cpu.registers[reg] = result;

    CpuFlags flags = 0;
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80u) flags |= FLAG_N;
    if (carry) flags |= FLAG_C;
    km8_cpu_set_flags(&ctx->cpu, flags);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_shr(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint8_t carry = (value & 0x01u) ? 1u : 0u;
    uint8_t result = (uint8_t)(value >> 1);
    ctx->cpu.registers[reg] = result;

    CpuFlags flags = 0;
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80u) flags |= FLAG_N;
    if (carry) flags |= FLAG_C;
    km8_cpu_set_flags(&ctx->cpu, flags);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_rol(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint8_t carry = (value & 0x80u) ? 1u : 0u;
    uint8_t result = (uint8_t)((value << 1) | (value >> 7));
    ctx->cpu.registers[reg] = result;

    CpuFlags flags = 0;
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80u) flags |= FLAG_N;
    if (carry) flags |= FLAG_C;
    km8_cpu_set_flags(&ctx->cpu, flags);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_ror(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);

    uint8_t value = ctx->cpu.registers[reg];
    uint8_t carry = (value & 0x01u) ? 1u : 0u;
    uint8_t result = (uint8_t)((value >> 1) | (value << 7));
    ctx->cpu.registers[reg] = result;

    CpuFlags flags = 0;
    if (result == 0) flags |= FLAG_Z;
    if (result & 0x80u) flags |= FLAG_N;
    if (carry) flags |= FLAG_C;
    km8_cpu_set_flags(&ctx->cpu, flags);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_tst_reg(Km8Context* ctx) {
    uint8_t reg0 = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg0);
    uint8_t reg1 = ctx->cpu.instr_buf[2];
    KM8_VALIDATE_REGISTER(reg1);

    (void)km8_cpu_alu_and(&ctx->cpu, ctx->cpu.registers[reg0], ctx->cpu.registers[reg1]);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_tst_imm(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER(reg);
    uint8_t imm = ctx->cpu.instr_buf[2];
    (void)km8_cpu_alu_and(&ctx->cpu, ctx->cpu.registers[reg], imm);
    return EXECUTION_SUCCESS;
}

// --- Control flow --------------------------------------------------------
static ExecutionResult op_jmp_mem(Km8Context* ctx) {
    uint16_t target = get_addr(ctx->cpu.instr_buf + 1);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jmp_reg(Km8Context* ctx) {
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jz_mem(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_Z))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jz_reg(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_Z))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnz_mem(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_Z))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnz_reg(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_Z))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jc_mem(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_C))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jc_reg(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_C))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnc_mem(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_C))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnc_reg(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_C))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jn_mem(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_N))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jn_reg(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_N))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnn_mem(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_N))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnn_reg(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_N))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jv_mem(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_V))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jv_reg(Km8Context* ctx) {
    if (!km8_cpu_is_flag_set(&ctx->cpu, FLAG_V))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnv_mem(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_V))
        return EXECUTION_SUCCESS;
    km8_cpu_set_pc(&ctx->cpu, get_addr(ctx->cpu.instr_buf + 1));
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_jnv_reg(Km8Context* ctx) {
    if (km8_cpu_is_flag_set(&ctx->cpu, FLAG_V))
        return EXECUTION_SUCCESS;
    uint8_t reg = ctx->cpu.instr_buf[1];
    KM8_VALIDATE_REGISTER_PAIR(reg);
    uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                      ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
    km8_cpu_set_pc(&ctx->cpu, target);
    return EXECUTION_SUCCESS;
}

static ExecutionResult op_call_mem(Km8Context* ctx) {
    switch (ctx->cpu.instr_exec_cycle) {
        case 0: {
            uint16_t pc = km8_cpu_get_pc(&ctx->cpu);
            stack_push(ctx, (uint8_t)(pc >> 8));
            ctx->cpu.value_buf = (uint8_t)(pc & 0xFF);
            return EXECUTION_PENDING;
        }
        case 1:
            stack_push(ctx, ctx->cpu.value_buf);
            return EXECUTION_PENDING;
        case 2: {
            uint16_t target = get_addr(ctx->cpu.instr_buf + 1);
            km8_cpu_set_pc(&ctx->cpu, target);
            return EXECUTION_SUCCESS;
        }
        default:
            return EXECUTION_FAILED;
    }
}

static ExecutionResult op_call_reg(Km8Context* ctx) {
    switch (ctx->cpu.instr_exec_cycle) {
        case 0: {
            uint16_t pc = km8_cpu_get_pc(&ctx->cpu);
            stack_push(ctx, (uint8_t)(pc >> 8));
            ctx->cpu.value_buf = (uint8_t)(pc & 0xFF);
            return EXECUTION_PENDING;
        }
        case 1:
            stack_push(ctx, ctx->cpu.value_buf);
            return EXECUTION_PENDING;
        case 2: {
            uint8_t reg = ctx->cpu.instr_buf[1];
            KM8_VALIDATE_REGISTER_PAIR(reg);
            uint16_t target = (uint16_t)ctx->cpu.registers[reg] |
                              ((uint16_t)ctx->cpu.registers[reg + 1] << 8);
            km8_cpu_set_pc(&ctx->cpu, target);
            return EXECUTION_SUCCESS;
        }
        default:
            return EXECUTION_FAILED;
    }
}

static ExecutionResult op_ret(Km8Context* ctx) {
    switch (ctx->cpu.instr_exec_cycle) {
        case 0:
            ctx->cpu.value_buf = stack_pop(ctx);
            return EXECUTION_PENDING;
        case 1: {
            uint8_t high = stack_pop(ctx);
            uint16_t addr = (uint16_t)ctx->cpu.value_buf | ((uint16_t)high << 8);
            km8_cpu_set_pc(&ctx->cpu, addr);
            return EXECUTION_SUCCESS;
        }
        default:
            return EXECUTION_FAILED;
    }
}

static ExecutionResult op_hlt(Km8Context* ctx) {
    ctx->cpu.state = CPU_HALT;
    ctx->cpu.halt_code = CPU_HALTCODE_MANUAL;
    return EXECUTION_SUCCESS;
}

static const Km8Opcode gOpcodeTable[256] = {
    [0x00] = OPCODE(op_nop,      1),
    [0x01] = OPCODE(op_ldr_mem,  4),
    [0x02] = OPCODE(op_ldr_imm,  3),
    [0x03] = OPCODE(op_str,      4),
    [0x04] = OPCODE(op_mov,      3),
    [0x05] = OPCODE(op_swp,      3),
    [0x06] = OPCODE(op_push,     2),
    [0x07] = OPCODE(op_pop,      2),
    [0x08] = OPCODE(op_clr,      2),
    /* 0x09-0x0F reserved */
    [0x10] = OPCODE(op_add_reg,  3),
    [0x11] = OPCODE(op_add_imm,  3),
    [0x12] = OPCODE(op_adc_reg,  3),
    [0x13] = OPCODE(op_adc_imm,  3),
    [0x14] = OPCODE(op_inc,      2),
    [0x15] = OPCODE(op_sub_reg,  3),
    [0x16] = OPCODE(op_sub_imm,  3),
    [0x17] = OPCODE(op_sbc_reg,  3),
    [0x18] = OPCODE(op_sbc_imm,  3),
    [0x19] = OPCODE(op_dec,      2),
    [0x1A] = OPCODE(op_cmp_reg,  3),
    [0x1B] = OPCODE(op_cmp_imm,  3),
    /* 0x1C-0x1F reserved */
    [0x20] = OPCODE(op_and_reg,  3),
    [0x21] = OPCODE(op_and_imm,  3),
    [0x22] = OPCODE(op_or_reg,   3),
    [0x23] = OPCODE(op_or_imm,   3),
    [0x24] = OPCODE(op_xor_reg,  3),
    [0x25] = OPCODE(op_xor_imm,  3),
    [0x26] = OPCODE(op_not,      2),
    [0x27] = OPCODE(op_shl,      2),
    [0x28] = OPCODE(op_shr,      2),
    [0x29] = OPCODE(op_rol,      2),
    [0x2A] = OPCODE(op_ror,      2),
    [0x2B] = OPCODE(op_tst_reg,  3),
    [0x2C] = OPCODE(op_tst_imm,  3),
    /* 0x2D-0x2F reserved */
    [0x30] = OPCODE(op_jmp_mem,  3),
    [0x31] = OPCODE(op_jmp_reg,  2),
    [0x32] = OPCODE(op_jz_mem,   3),
    [0x33] = OPCODE(op_jz_reg,   2),
    [0x34] = OPCODE(op_jnz_mem,  3),
    [0x35] = OPCODE(op_jnz_reg,  2),
    [0x36] = OPCODE(op_jc_mem,   3),
    [0x37] = OPCODE(op_jc_reg,   2),
    [0x38] = OPCODE(op_jnc_mem,  3),
    [0x39] = OPCODE(op_jnc_reg,  2),
    [0x3A] = OPCODE(op_jn_mem,   3),
    [0x3B] = OPCODE(op_jn_reg,   2),
    [0x3C] = OPCODE(op_jnn_mem,  3),
    [0x3D] = OPCODE(op_jnn_reg,  2),
    [0x3E] = OPCODE(op_jv_mem,   3),
    [0x3F] = OPCODE(op_jv_reg,   2),
    [0x40] = OPCODE(op_jnv_mem,  3),
    [0x41] = OPCODE(op_jnv_reg,  2),
    [0x42] = OPCODE(op_call_mem, 3),
    [0x43] = OPCODE(op_call_reg, 2),
    [0x44] = OPCODE(op_ret,      1),
    [0x45] = OPCODE(op_hlt,      1),
    /* 0x46-0xFF reserved */
};
