#include "platform.h"
#include "cpu.h"
#include "emulator.h"

#include <inttypes.h>

uint64_t last_time = 0;

void Init() {
    CpuInit();
    
    /*
    last_time = now_nanos();


    uint64_t tS = now_nanos();

    uint64_t dtT = now_nanos();

    printf("Starting profiling...\n");
    for (int i = 0; i < 1000000000; i++) {
        //gGame->UpdateFrame(0);
        //EmuStepCycle();

        uint64_t now = now_nanos();
        uint64_t dt = now - dtT;
        dtT = now;

        EmuUpdate(dt);
    }
    uint64_t tE = now_nanos();

    printf("Elapsed: %" PRIu64 " ns\n", tE - tS);
    printf("NS per cycle: %.2f ns\n", (double)(tE - tS) / gTotalCycles);
    printf("Total Cycles: %" PRIu64 "\n", gTotalCycles);
    */
}

void KeyEvent(uint8_t key, uint8_t state) {

}

void UpdateFrame() {
    uint64_t now = now_nanos();
    uint64_t delta = now - last_time;
    last_time = now;

    // If window was moved, reset delta to 0 (optional logic here)
    if (delta > MAX_CYCLE_DELTA) {
        delta = 0;
        //command_out("Running behind schedule!");
    }

    EmuUpdate(delta);
}

    

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {

    FILE* fp;
    AllocConsole();
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    CreateThread(NULL, 0, ConsoleThread, NULL, 0, NULL);

    Game game = { Init, UpdateFrame, KeyEvent };
    PlatformInit(hInstance, nCmdShow, &game);
}