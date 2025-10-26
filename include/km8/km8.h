#pragma once

#include <stdint.h>

#include "cartridge.h"
#include "cpu.h"

#define WRAM_BANK_COUNT 8
#define WRAM_SIZE (SIZE_WRAM0 + SIZE_WRAMN)

typedef enum {
    KM8_OK = 0,
    KM8_BUS_RANGE_OVERLAP,
    KM8_INVALID_ROM,
    KM8_FILE_NOT_FOUND
} Km8Result;

typedef enum {
    CPU_NONE = 0,
    CPU_INVALID_OPCODE
} Km8HaltCode;

typedef struct Km8Context {
    uint16_t programCounter;
    uint64_t cycles;

    uint8_t wram[WRAM_SIZE];
    
    Cpu cpu;
    Cartridge* cartridge;
} Km8Context;

void km8_init(void);
Km8Context km8_create_context(void);

void km8_step_cycles(Km8Context* context, uint64_t count);
void km8_step_instructions(Km8Context* context, uint64_t count);
void km8_step_lines(Km8Context* context, uint64_t count);
void km8_step_frames(Km8Context* context, uint64_t count);
