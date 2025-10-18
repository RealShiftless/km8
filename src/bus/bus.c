#include "bus.h"

#include "km8/km8.h"

#define KM8_RETURN_IF_ERROR(expr)         \
    do {                                  \
        Km8Result _km8_result = (expr);   \
        if (_km8_result != KM8_OK) {      \
            return _km8_result;           \
        }                                 \
    } while (0)

Km8Result km8_bus_init() {
    KM8_RETURN_IF_ERROR(km8_rom_register_device());
    KM8_RETURN_IF_ERROR(km8_eram_register_device());

    return KM8_OK;
}

Km8BusAccess km8_bus_read(Km8Context* ctx, uint16_t address) {
    BusDevice* device = km8_bus_device_resolve(address);
    if(!device) {
        return (Km8BusAccess){0};
    }

    uint16_t local_address = km8_bus_localize_address(device, address);

    uint8_t latency = device->get_latency(local_address);
    uint8_t value   = device->on_read(ctx, local_address);

    return (Km8BusAccess){
        .value = value,
        .latency_cycles = latency
    };
}

Km8BusAccess km8_bus_write(Km8Context* ctx, uint16_t address, uint8_t value) {
    BusDevice* device = km8_bus_device_resolve(address);
    if(!device) {
        return (Km8BusAccess){0};
    }

    uint16_t local_address = km8_bus_localize_address(device, address);

    uint8_t latency = device->get_latency(local_address);
    device->on_write(ctx, local_address, value);

    return (Km8BusAccess) {
        .latency_cycles = latency
    };
}
