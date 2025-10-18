#pragma once

#include <stdint.h>
#include "km8/km8.h"

// ─────────────────────────────────────────────
// KM8 Bus Address Map
// ─────────────────────────────────────────────

#define ADDRESS_RANGE      0x10000

// BIOS
#define ADDR_BIOS_START   0x0000
#define ADDR_BIOS_END     0x00FF

// Fixed ROM bank (ROM0)
#define ADDR_ROM0_START   0x0100
#define ADDR_ROM0_END     0x40FF

// Switchable ROM bank (ROMn)
#define ADDR_ROMN_START   0x4100
#define ADDR_ROMN_END     0x80FF

// Work RAM 0 (fast internal)
#define ADDR_WRAM0_START  0x8100
#define ADDR_WRAM0_END    0xA0FF

// Work RAM n (banked external)
#define ADDR_WRAMN_START  0xA100
#define ADDR_WRAMN_END    0xC0FF

// Video RAM
#define ADDR_VRAM_START   0xC100
#define ADDR_VRAM_END     0xE0FF

// External (cartridge) RAM
#define ADDR_ERAM_START   0xE100
#define ADDR_ERAM_END     0xF0FF

// Object Attribute Memory (sprites)
#define ADDR_OAM_START    0xF100
#define ADDR_OAM_END      0xF1FF

// I/O registers
#define ADDR_IO_START     0xF200
#define ADDR_IO_END       0xF2FF

// High-speed scratchpad RAM
#define ADDR_HRAM_START   0xF300
#define ADDR_HRAM_END     0xF3FF

// Unused / open bus region
#define ADDR_UNUSED_START 0xF400
#define ADDR_UNUSED_END   0xFFFF

// ─────────────────────────────────────────────
// Derived sizes
// ─────────────────────────────────────────────
#define SIZE_BIOS   (ADDR_BIOS_END   - ADDR_BIOS_START   + 1)
#define SIZE_ROM0   (ADDR_ROM0_END   - ADDR_ROM0_START   + 1)
#define SIZE_ROMN   (ADDR_ROMN_END   - ADDR_ROMN_START   + 1)
#define SIZE_WRAM0  (ADDR_WRAM0_END  - ADDR_WRAM0_START  + 1)
#define SIZE_WRAMN  (ADDR_WRAMN_END  - ADDR_WRAMN_START  + 1)
#define SIZE_VRAM   (ADDR_VRAM_END   - ADDR_VRAM_START   + 1)
#define SIZE_ERAM   (ADDR_ERAM_END   - ADDR_ERAM_START   + 1)
#define SIZE_OAM    (ADDR_OAM_END    - ADDR_OAM_START    + 1)
#define SIZE_IO     (ADDR_IO_END     - ADDR_IO_START     + 1)
#define SIZE_HRAM   (ADDR_HRAM_END   - ADDR_HRAM_START   + 1)
#define SIZE_UNUSED (ADDR_UNUSED_END - ADDR_UNUSED_START + 1)

// ─────────────────────────────────────────────
// Latencies
// ─────────────────────────────────────────────
#define LATENCY_BIOS  1
#define LATENCY_ROM0  4
#define LATENCY_ROMN  5
#define LATENCY_WRAM0 1
#define LATENCY_WRAMN 2
#define LATENCY_VRAM  2
#define LATENCY_ERAM  8
#define LATENCY_ORAM  2
#define LATENCY_HRAM  1


// Structs
typedef struct {
    uint16_t start;
    uint16_t end; // Inclusive

    uint8_t(*on_read)(Km8Context* ctx, uint16_t address);
    void(*on_write)(Km8Context* ctx, uint16_t address, uint8_t data);

    uint8_t(*get_latency)(uint16_t address);
} BusDevice;

typedef struct {
    uint8_t value;
    uint8_t latency_cycles;
} Km8BusAccess;

// Initialization functions
Km8Result km8_bus_init();

Km8Result km8_rom_register_device();
Km8Result km8_eram_register_device();

// Runtime Func
Km8BusAccess km8_bus_read(Km8Context* ctx, uint16_t address);
Km8BusAccess km8_bus_write(Km8Context* ctx, uint16_t address, uint8_t value);

BusDevice* km8_bus_device_resolve(uint16_t address);
Km8Result km8_bus_device_register(BusDevice* device);

// Helpers
static inline uint16_t km8_bus_localize_address(const BusDevice* dev, uint16_t addr) {
    return addr - dev->start;
}