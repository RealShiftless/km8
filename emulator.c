#include "emulator.h"
#include "cpu.h"
#include "cli.h"
#include "platform.h"

#include <inttypes.h>

// Interface
Cartridge gCartridge;

//uint8_t gEmulatorFlags;
uint8_t gExecutionMode;
uint64_t gTotalCycles;

uint64_t gCurCycle; 

// Memory
static uint8_t gBios[BIOS_SIZE] = {
    0x02, 0x07, 0xFF, 0x02, 0x08, 0xF2, 0x40, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t gRam[RAM_BANKS][RAM_BANK_SIZE];
static uint8_t gHRam[HRAM_SIZE];

// Timing
static uint32_t gFrameCycles;

// Emulator func
void get_haltcode_string(uint8_t haltcode, char* haltcodeString, size_t bufferSize) {
    const char* str;

    switch (haltcode) {
        case HALTCODE_MANUAL:            str = "Manual"; break;
        case HALTCODE_INVALID_OPCODE:    str = "Invalid Opcode"; break;
        case HALTCODE_INVALID_OPERAND:   str = "Invalid Operand"; break;
        case HALTCODE_TRAP:              str = "Trap"; break;
        case HALTCODE_STACK_OVERFLOW:    str = "Stack Overflow"; break;
        case HALTCODE_BUS_FAULT:         str = "Bus Fault"; break;
        case HALTCODE_UNKNOWN:           str = "Unknown"; break;
        case HALTCODE_WRITE_PROTECTED:   str = "Write Protected"; break;
        case HALTCODE_READ_PROTECTED:    str = "Read Protected"; break;
        default:                         str = "???"; break;
    }

    snprintf(haltcodeString, bufferSize, "%s", str);
}

void init_emulation() {
    gExecutionMode = EXEC_PAUSED;
    gTotalCycles = 0;
    gCurCycle = 0;
    gFrameCycles = 0;

    init_cpu();
}

void exec_idle() {
}

void exec_normal() {
    gFrameCycles = CYCLES_PER_FRAME;

    while (gFrameCycles-- > 0)
    {
        gTotalCycles++;
        cpu_run_cycle();
    }
}

void exec_paused() {

}

void exec_stepped() {

}

void emu_update_frame() {
    cli_process_input();

    switch (gExecutionMode)
    {
        case EXEC_IDLE: exec_idle(); return;
        case EXEC_NORMAL: exec_normal(); return;
        case EXEC_PAUSED: exec_paused(); return;
        case EXEC_STEPPED: exec_stepped(); return;
    // Add default error handling
    }
}

void emu_stop() {
    gExecutionMode = EXEC_IDLE;

    gFrameCycles = 0;
}

void emu_halt() {
    emu_stop();

    if (gCpuHaltCode != HALTCODE_MANUAL) {
        char haltcodeBuffer[16];
        get_haltcode_string(gCpuHaltCode, &haltcodeBuffer, 16);
        cli_runtime_err_out("Halted (HC: %s, PC: %04X)", haltcodeBuffer, get_pc());
    }
    else {
        cli_runtime_out("Halted (HC: Manual, PC: %04X)", get_pc());
    }
}

void free_cartridge(Cartridge* cartridge, uint8_t romSize, uint8_t ramSize) {
    for (int i = 0; i < romSize; i++)
        free(cartridge->rom[i]);
    free(cartridge->rom);

    for (int i = 0; i < ramSize; i++)
        free(cartridge->ram[i]);
    free(cartridge->ram);

    memset(cartridge, 0, sizeof(*cartridge));
}

uint8_t unload_rom(void) {
    free_cartridge(&gCartridge, gCartridge.header.rom_size, gCartridge.header.ram_size);
}

uint8_t load_rom(const char* path) {
    if (gCartridge.rom != NULL)
        unload_rom();

    FILE* file = fopen(path, "rb");
    if (!file) {
        return LOAD_FAILED;
    }

    // First read the static rom area...
    size_t bytesRead = fread(&gCartridge.header, 1, CART_HEADER_S, file);

    // Validate the header
    if (bytesRead != CART_HEADER_S) { // Checking size
        fclose(file);
        return LOAD_INVALLID_HEADER;
    }

    // Validate the magic
    if (strncmp(gCartridge.header.magic, "KM8", 3) != 0) {
        fclose(file);
        return LOAD_INVALLID_HEADER;
    }

    // Validate rom sizing
    if (gCartridge.header.rom_size == 0)
    {
        fclose(file);
        return LOAD_INVALLID_HEADER;
    }

    // Reset back to start :)
    fseek(file, 0, SEEK_SET);

    // Allocate ROM banks
    gCartridge.rom = malloc(gCartridge.header.rom_size * sizeof(uint8_t*));

    // Check if allocation was succesful
    if (!gCartridge.rom) {
        free_cartridge(&gCartridge, 0, 0);

        fclose(file);
        return LOAD_ALLOC_FAILED;
    }

    // Initialize the ROM banks
    for (int i = 0; i < gCartridge.header.rom_size; i++) {
        gCartridge.rom[i] = malloc(CART_ROM_BANK_S);

        // Check if we allocated succesfuly
        if (!gCartridge.rom[i]) {
            free_cartridge(&gCartridge, i, 0);

            fclose(file);
            return LOAD_ALLOC_FAILED;
        }
    }

    // Allocate RAM banks
    gCartridge.ram = malloc(gCartridge.header.ram_size * sizeof(uint8_t*));

    // Check if alloc was succesful
    if (!gCartridge.ram) {
        free_cartridge(&gCartridge, gCartridge.header.rom_size, 0);

        fclose(file);
        return LOAD_ALLOC_FAILED;
    }

    // Initialize the entire ram with 0's
    for (int i = 0; i < gCartridge.header.ram_size; i++) {
        gCartridge.ram[i] = calloc(CART_RAM_BANK_S, 1);

        // Check if we allocated succesfuly
        if (!gCartridge.ram[i]) {
            free_cartridge(&gCartridge, gCartridge.header.rom_size, i);

            fclose(file);
            return LOAD_ALLOC_FAILED;
        }
    }

    // Load the rom into memory
    for (int i = 0; i < gCartridge.header.rom_size; i++) {
        size_t read = fread(gCartridge.rom[i], 1, CART_ROM_BANK_S, file);
        if (read != CART_ROM_BANK_S) {
            cli_warn("Bank %d only got %zu bytes", i, read);
        }
    }

    // Close file, set flag and return :)
    fclose(file);

    return LOAD_SUCCESS;
}

// Read Bios
uint8_t read_bios(uint8_t address) {
    return gBios[address];
}

// Read rom
uint8_t read_rom(uint8_t bank, uint16_t address) {
    return gCartridge.rom[bank][address];
}
uint8_t read_rom_flat(uint32_t globalAddress) {
    uint8_t bank = (uint8_t)(globalAddress / CART_ROM_BANK_S);
    uint16_t address = (uint16_t)(globalAddress % CART_ROM_BANK_S);

    return read_rom(bank, address);
}

// Read internal ram
uint8_t read_iram(uint8_t bank, uint16_t address) {
    return gRam[bank][address];
}
uint8_t read_iram_flat(uint32_t globalAddress) {
    uint8_t bank = (uint8_t)(globalAddress / RAM_BANK_SIZE);
    uint16_t address = (uint16_t)(globalAddress % RAM_BANK_SIZE);

    return read_iram(bank, address);
}

// Write to internal ram
void write_iram(uint8_t bank, uint16_t address, uint8_t value) {
    gRam[bank][address] = value;
}
void write_iram_flat(uint32_t globalAddress, uint8_t value) {
    uint8_t bank = (uint8_t)(globalAddress / RAM_BANK_SIZE);
    uint16_t address = (uint16_t)(globalAddress % RAM_BANK_SIZE);

    write_iram(bank, address, value);
}

// Read external ram
uint8_t read_eram(uint8_t bank, uint16_t address) {
    if (gCartridge.ram == NULL || gCartridge.header.ram_size == 0)
        return 0xFF;

    return gCartridge.ram[bank][address];
}
uint8_t read_eram_flat(uint32_t globalAddress) {
    if (gCartridge.ram == NULL || gCartridge.header.ram_size == 0)
        return 0xFF;

    uint8_t bank = (uint8_t)(globalAddress / CART_RAM_BANK_S);
    uint16_t address = (uint16_t)(globalAddress % CART_RAM_BANK_S);

    return read_eram(bank, address);
}

// Write to external ram
void write_eram(uint8_t bank, uint16_t address, uint8_t value) {
    gCartridge.ram[bank][address] = value;
}
void write_eram_flat(uint32_t globalAddress, uint8_t value) {
    uint8_t bank = (uint8_t)(globalAddress / RAM_BANK_SIZE);
    uint16_t address = (uint16_t)(globalAddress % RAM_BANK_SIZE);

    write_eram(bank, address, value);
}


// Hram
uint8_t read_hram(uint8_t address) {
    return gHRam[address];
}
void write_hram(uint8_t address, uint8_t value) {
    gHRam[address] = value;
}
