#include <stdint.h>

typedef enum {
	EMUFLAG_NONE = 0b00000000,
	EMUFLAG_RUNNING = 0b00000001,
	EMUFLAG_ROMLOADED = 0b00000010
} EmulatorFlags;

typedef enum {
	EXEC_NORMAL        = 0b00000000,
	EXEC_PAUSED        = 0b00000001,
	EXEC_FRAME_STEPPED = 0b00000010,
	EXEC_CYCLE_STEPPED = 0b00000011
} ExecutionMode;

extern uint8_t gEmulatorFlags;
extern uint8_t gExecutionMode;
extern uint64_t gTotalCycles;

void EmuInit(void);
void EmuUpdate(uint64_t delta);
void EmuStepCycle(void);

#pragma once