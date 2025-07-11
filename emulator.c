#include "emulator.h"
#include "cpu.h"
#include "console.h"
#include "platform.h"

#include <inttypes.h>

uint8_t gEmulatorFlags;
uint8_t gExecutionMode;
uint64_t gTotalCycles;

uint64_t leftover = 0;

uint8_t resetTiming = 0;

void EmuInit() {
}

void EmuUpdate(uint64_t delta) {
	if ((gEmulatorFlags & EMUFLAG_RUNNING) == 0)
		return;


    if (gExecutionMode == EXEC_NORMAL) {
        // Update the left over time
        leftover += delta;

        // Process a cycle if needed :)
        if (leftover >= CLOCK_CYCLE_NS) {
            leftover -= CLOCK_CYCLE_NS;
            EmuStepCycle();

            if (leftover > 10000000) {
                printf("out of time!!!");
                leftover = 0;
            }
        }
    }
}

uint64_t cycleT;
void EmuStepCycle() {
    gTotalCycles++;
    CpuStep();
}