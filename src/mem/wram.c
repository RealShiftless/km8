#include "km8/km8.h"

#include <stddef.h>

#include "bus.h"

static uint8_t gCurBank = 0;

enum {
    KM8_WRAM_BANK_COUNT = 8, /* 8 KiB windows => 8 banks cover 64 KiB WRAM */
    KM8_WRAM_SIZE = SIZE_WRAM0 + SIZE_WRAMN
};

static size_t wram_resolve_offset(uint16_t local_address) {
    if (local_address < SIZE_WRAM0) {
        return (size_t)local_address;
    }

    size_t offset_in_bank = (size_t)(local_address - SIZE_WRAM0);
    size_t bank_offset = ((size_t)gCurBank % KM8_WRAM_BANK_COUNT) * SIZE_WRAMN;
    return SIZE_WRAM0 + ((bank_offset + offset_in_bank) % SIZE_WRAMN);
}

static uint8_t on_read(Km8Context* ctx, uint16_t local_address) {
    size_t offset = wram_resolve_offset(local_address);
    return ctx->wram[offset % KM8_WRAM_SIZE];
}

static void on_write(Km8Context* ctx, uint16_t local_address, uint8_t value) {
    size_t offset = wram_resolve_offset(local_address);
    ctx->wram[offset % KM8_WRAM_SIZE] = value;
}

static uint8_t get_latency(uint16_t local_address) {
    return (local_address < SIZE_WRAM0) ? LATENCY_WRAM0 : LATENCY_WRAMN;
}

static BusDevice gDevice = {
    .start = ADDR_WRAM0_START,
    .end = ADDR_WRAMN_END,
    .on_read = on_read,
    .on_write = on_write,
    .get_latency = get_latency
};

Km8Result km8_wram_register_device(void) {
    return km8_bus_device_register(&gDevice);
}

void km8_wram_select_bank(uint8_t bank) {
    gCurBank = bank;
}
