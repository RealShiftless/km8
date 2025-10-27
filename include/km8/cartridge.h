#pragma once

#include <stdint.h>

#define ROM_BANK_SIZE 0x4000
#define ERAM_BANK_SIZE 0x1000

typedef struct {
    char magic[4];   // Magic        Header: 0x0000..0x0003

    uint8_t rom_size_code;      // Rom Banks    Header: 0x0004
    uint32_t rom_bank_count;

    uint8_t ram_size_code;
    uint32_t ram_bank_count;      // Ram Banks    Header: 0x0005

    uint8_t flags;          // Flags        Header: 0x0006

    char title[10];  // Title        Header: 0x0007..0x000F

    uint8_t* rom;
    uint8_t* ram;
} Cartridge;

Cartridge* km8_load_cartridge(uint8_t rom_data[], uint32_t size);
void km8_unload_cartridge(Cartridge* cartridge);

//uint8_t km8_cartridge_read_rom(const Cartridge* cartridge, uint16_t offset);
//uint8_t km8_cartridge_read_ram(const Cartridge* cartridge, uint16_t offset);
