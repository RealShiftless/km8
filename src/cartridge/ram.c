#include "km8/km8.h"

#include "bus.h"

// Values
static uint8_t gCurBank;

// Helper func
static inline uint32_t map_address(uint8_t banks, uint16_t address) {
    // Map into banked space
    address += gCurBank * SIZE_ERAM;
    
    // Fix overflow
    address %= SIZE_ERAM * banks;

    return address;
}

// Device func
static uint8_t on_read(Km8Context* ctx, uint16_t address) {
    if(ctx->cartridge->ram_banks == 0) {
        return 0;
    }

    uint32_t mapped_address = map_address(ctx->cartridge->ram_banks, address);

    return ctx->cartridge->ram[mapped_address];
}
static void on_write(Km8Context* ctx, uint16_t address, uint8_t value) {
    if(ctx->cartridge->ram_banks == 0) {
        return;
    }

    uint32_t mapped_address = map_address(ctx->cartridge->ram_banks, address);

    ctx->cartridge->ram[mapped_address] = value;
}

static uint8_t get_latency(uint16_t address) {
    return LATENCY_ERAM;
}

static BusDevice gDevice = {
    .start = ADDR_ERAM_START,
    .end = ADDR_ERAM_END,

    .on_read = on_read,
    .on_write = on_write,
    .get_latency = get_latency
};

// Register the device
Km8Result km8_eram_register_device() {
    return km8_bus_device_register(&gDevice);
}
