#include "bus.h"
#include "cpu_internal.h"

void km8_init() {
    km8_bus_init();
}

Km8Context km8_create_context() {
    Km8Context ctx = {0};

    return ctx;
}

void km8_step_cycles(Km8Context* ctx, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        km8_cpu_step(ctx);

        // TODO: Add ppu integration
    }
}
void km8_step_instructions(Km8Context* ctx, uint64_t count) {
    // TODO: Impelement debugging step instruction
}
void km8_step_lines(Km8Context* ctx, uint64_t count) {

}
void km8_step_frames(Km8Context* ctx, uint64_t count) {

}