#include "km8/cartridge.h"

#include <stdlib.h>
#include <string.h>

//                                                K     M     8     █
static const uint8_t CART_HEADER_MAGIC [4] = {0x4B, 0x4D, 0x38, 0xB0};

static const uint32_t ROM_SIZE_CODES[] = { 
    2, 
    4, 
    8, 
    16, 
    32, 
    64, 
    128, 
    256, 
    512
};

static const uint16_t RAM_SIZE_CODES[] = {
    0,
    1,
    4,
    16,
    32,
    64
};

Cartridge* km8_load_cartridge(uint8_t rom_data[], uint32_t size) {
    // First check if this byte array can even be a cartridge
    if(size < ROM_BANK_SIZE || size % ROM_BANK_SIZE) {
        return NULL;
    }

    // Load the header into memory
    Cartridge* cartridge = calloc(1, sizeof(Cartridge));

    if(cartridge == NULL) {
        return NULL;
    }

    // Check the cartridge magic
    memcpy(cartridge->magic, rom_data, 4);
    
    if(memcmp(cartridge->magic, CART_HEADER_MAGIC, 4)) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }

    // Load the rom size from the rom_size_code in the header
    cartridge->rom_size_code = rom_data[4];
    size_t rom_size_code_count = sizeof(ROM_SIZE_CODES) / sizeof(ROM_SIZE_CODES[0]);
    if(cartridge->rom_size_code >= rom_size_code_count) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }
    cartridge->rom_bank_count = ROM_SIZE_CODES[cartridge->rom_size_code];
    
    uint32_t rom_bytes = cartridge->rom_bank_count * ROM_BANK_SIZE;
    if(rom_bytes != size) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }

    // Load the ram size from the ram_size_code in the header
    cartridge->ram_size_code = rom_data[5];
    size_t ram_size_code_count = sizeof(RAM_SIZE_CODES) / sizeof(RAM_SIZE_CODES[0]);
    if(cartridge->ram_size_code >= ram_size_code_count) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }
    cartridge->ram_bank_count = RAM_SIZE_CODES[cartridge->ram_size_code];

    cartridge->flags     = rom_data[6];

    // Copy the title
    memcpy(cartridge->title, rom_data + 7, 9);

    // Initialize the rom area
    cartridge->rom = calloc(cartridge->rom_bank_count * ROM_BANK_SIZE, 1);
    if(cartridge->rom == NULL) {
        km8_unload_cartridge(cartridge);
        return NULL;
    }
    memcpy(cartridge->rom, rom_data, size);

    // Initialize the ram area
    if(cartridge->ram_bank_count > 0) {
        cartridge->ram = calloc(cartridge->ram_bank_count * ERAM_BANK_SIZE, 1);
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
