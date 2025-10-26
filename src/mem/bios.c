#include "km8/km8.h"

#include "bus.h"

// Values
static uint8_t gBios[256] = {
    // LDR r7 0xFF (Low byte for SP)
    0x02, 0x07, 0xFF,

    // LDR r8 0xA0 (High byte for SP)
    0x02, 0x08, 0xA0,

    // JMP $0x0110
    0x30, 0x10, 0x01
};

// Device func
static uint8_t on_read(Km8Context* ctx, uint16_t local_address) {
    return gBios[local_address];
}
static void on_write(Km8Context* ctx, uint16_t local_address, uint8_t value) { /* READ ONLY */ }

static uint8_t get_latency(uint16_t address) {
    return LATENCY_BIOS;
}

static BusDevice gDevice = {
    .start = ADDR_BIOS_START,
    .end = ADDR_BIOS_END,

    .on_read = on_read,
    .on_write = on_write,
    .get_latency = get_latency
};

// Register the device
Km8Result km8_bios_register_device() {
    return km8_bus_device_register(&gDevice);
}
