#include "bus.h"

#include "km8/km8.h"

static BusDevice* gDispatchTable[ADDRESS_RANGE] = {0};

BusDevice* km8_bus_device_resolve(uint16_t address) {
    return gDispatchTable[address];
}

Km8Result km8_bus_device_register(BusDevice* device) {
    for(int i = device->start; i <= device->end; i++) {
        if(gDispatchTable[i] != NULL)
            return KM8_BUS_RANGE_OVERLAP;

        gDispatchTable[i] = device;
    }

    return KM8_OK;
}