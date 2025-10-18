#include "km8/cartridge.h"

#include <stdlib.h>
#include <string.h>
//                               K     M     8     █
static const uint8_t MAGIC[4] = {0x4B, 0x4D, 0x38, 0xB0};

Cartridge* km8_load_cartridge(uint8_t rom_data[], uint32_t size) {
    if(size < ROM_BANK_SIZE) {
        return NULL;
    }

    // Load the header into memory
    Cartridge* cartridge = calloc(1, sizeof(Cartridge));

    if(cartridge == NULL) {
        return NULL;
    }

    memcpy(cartridge->magic, rom_data, 4);

    if(memcmp(cartridge->magic, MAGIC, 4)) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }
    
    cartridge->rom_banks = rom_data[4];
    if(cartridge->rom_banks * ROM_BANK_SIZE != size) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }

    cartridge->ram_banks = rom_data[5];
    cartridge->flags     = rom_data[6];

    memcpy(cartridge->title, rom_data + 7, 9);
    // Initialize the memory spaces for the cartridge
    cartridge->rom = calloc(cartridge->rom_banks * ROM_BANK_SIZE, 1);
    if(cartridge->rom == NULL) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }

    memcpy(cartridge->rom, rom_data, size);
    if(cartridge->ram_banks > 0) {
        cartridge->ram = calloc(cartridge->ram_banks * ERAM_BANK_SIZE, 1);
        if(cartridge->ram == NULL) {
            km8_unload_cartridge(cartridge);
            return NULL;
        }
    }

    return cartridge;
}

void km8_unload_cartridge(Cartridge* cartridge) {
    if(!cartridge) {
        return;
    }
    
    if(cartridge->rom) {
        free(cartridge->rom);
    }

    if(cartridge->ram) {
        free(cartridge->ram);
    }
    
    free(cartridge);
}