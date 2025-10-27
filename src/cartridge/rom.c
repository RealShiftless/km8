#include "km8/km8.h"

#include "bus.h"

// Values
static uint8_t gCurBank;

// Device func
static uint8_t on_read(Km8Context* ctx, uint16_t address) {
    if(address < SIZE_ROM0) {
        return ctx->cartridge->rom[address];
    }
    else if (address - SIZE_ROM0 < SIZE_ROMN) {
        uint32_t full_address = address - SIZE_ROM0 + SIZE_ROMN * gCurBank;

        return ctx->cartridge->rom[full_address];
    }

    return 0;
}
static void on_write(Km8Context* ctx, uint16_t address, uint8_t value) { /* READ ONLY */ }

static uint8_t get_latency(uint16_t address) {
    if(address < SIZE_ROM0) {
        return LATENCY_ROM0;
    }
    else if(address - SIZE_ROM0 < SIZE_ROMN) {
        return LATENCY_ROMN;
    }

    return 0;
}

static BusDevice gDevice = {
    .start = ADDR_ROM0_START,
    .end = ADDR_ROMN_END,

    .on_read = on_read,
    .on_write = on_write,
    .get_latency = get_latency
};

// Register the device
Km8Result km8_rom_register_device() {
    return km8_bus_device_register(&gDevice);
}
