#include "platform.h"
#include "cpu.h"
#include "emulator.h"

#include <inttypes.h>

uint64_t last_time = 0;

void init() {
    cpu_init();
    emu_init();
    last_time = now_nanos();

    /*
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);


    load_rom("nop.bin");
    gCpuCurrState = CPU_FETCH_OPCODE;
    gProgramCounter = 0x0010;

    QueryPerformanceCounter(&start);
    const uint64_t perfCycles = CYCLES_PER_FRAME;
    for (int i = 0; i < perfCycles; i++)
    {
        cpu_run_cycle();
    }

    QueryPerformanceCounter(&end); // End timing

    double elapsed_ns = (double)(end.QuadPart - start.QuadPart) * 1e9 / freq.QuadPart;

    printf("Executed %d cycles in %.2f ns (%.5f s)\n", perfCycles, elapsed_ns, elapsed_ns / 1000000000.0);
    printf("Average per cycle: %.2f ns\n", elapsed_ns / perfCycles);
    */

    /*


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
void on_exit() {
    cli_exit();
}

void key_event(uint8_t key, uint8_t state) {

}

void update_frame() {
    emu_update_frame();
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

    CreateThread(NULL, 0, cli_thread_func, NULL, 0, NULL);

    Game game = { init, on_exit, update_frame, key_event };
    platform_init(hInstance, nCmdShow, &game);
}