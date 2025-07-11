#include "rom.h"
#include "console.h"

uint8_t rom[ROM_SIZE];

uint8_t RomRead(uint16_t address) {
	return rom[address];
}

uint8_t RomLoad(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return LOAD_FAILED; // failure
    }

    size_t bytesRead = fread(rom, 1, ROM_SIZE, file);
    fclose(file);

    if (bytesRead != ROM_SIZE) {
        command_out("Warning: Expected 64KB, got %zu bytes\n", bytesRead);
        return LOAD_SIZEWARN;
    }

    return LOAD_SUCCESS;
}