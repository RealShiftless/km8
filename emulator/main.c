#ifndef _WIN32
#define _POSIX_C_SOURCE 199309L
#endif

#include "platform/platform.h"

#include "km8/km8.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <time.h>
#include <termios.h>
#include <unistd.h>
#endif

#define KM8_FRAME_WIDTH        256
#define KM8_FRAME_HEIGHT       144
#define KM8_GRID_CELL            8
#define KM8_TARGET_SCALE         4
#define KM8_FRAME_INTERVAL_NS 16666667ULL


static void km8_wait_for_any_key(void) {
    puts("Press any key to continue...");
#if defined(_WIN32)
    _getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

static void fill_black(uint32_t* pixels, uint32_t width, uint32_t height) {
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            pixels[y * width + x] = 0x0;
        }
    }
}

static void fill_mem_view(Km8Context* ctx, uint32_t* pixels, uint32_t width, uint32_t height) {
    for(int i = 0; i < REGISTER_COUNT; i++) {
        uint8_t value = ctx->cpu.registers[i];
        uint32_t color = ((255 - value) << 16) | value;

        int x = i * 2 + 1;
        pixels[1 * width + x] = color;
    }

    for(int i = 0; i < 8192; i++) {
        uint8_t value = ctx->wram[i];
        uint32_t color = ((255 - value) << 16) | value;

        int x = (i % 128) * 2 + 1;
        int y = 3 + (i / 128) * 2;
        pixels[y * width + x] = color;
    }
}

static uint64_t km8_get_time_ns(void) {
#if defined(_WIN32)
    static double gCounterToNs = 0.0;
    if (gCounterToNs == 0.0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        gCounterToNs = 1e9 / (double)freq.QuadPart;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)(counter.QuadPart * gCounterToNs);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

static void km8_sleep_ns(uint64_t ns) {
    if (ns == 0) {
        return;
    }

#if defined(_WIN32)
    DWORD ms = (DWORD)((ns + 999999ULL) / 1000000ULL);
    if (ms == 0) {
        Sleep(0);
    } else {
        Sleep(ms);
    }
#else
    struct timespec ts;
    ts.tv_sec  = (time_t)(ns / 1000000000ULL);
    ts.tv_nsec = (long)(ns % 1000000000ULL);
    nanosleep(&ts, NULL);
#endif
}

static uint8_t* km8_read_entire_file(const char* path, size_t* out_size) {
    if (out_size) {
        *out_size = 0;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    uint8_t* data = (uint8_t*)malloc((size_t)file_size);
    if (!data) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(data, 1, (size_t)file_size, file);
    fclose(file);

    if (read != (size_t)file_size) {
        free(data);
        return NULL;
    }

    if (out_size) {
        *out_size = (size_t)file_size;
    }

    return data;
}

int main(int argc, char** argv) {
    if (!km8_platform_init()) {
        fprintf(stderr, "Failed to initialize platform layer.\n");
        return EXIT_FAILURE;
    }

    char* owned_rom_path = NULL;
    const char* rom_path = NULL;
    if (argc >= 2) {
        rom_path = argv[1];
    } else {
        owned_rom_path = km8_platform_open_rom_dialog();
        rom_path = owned_rom_path;
    }

    if (!rom_path) {
        fprintf(stderr, "No ROM selected.\n");
        km8_platform_shutdown();
        return EXIT_FAILURE;
    }

    printf("Selected ROM: %s\n", rom_path);

    size_t rom_size = 0;
    uint8_t* rom_bytes = km8_read_entire_file(rom_path, &rom_size);
    if (!rom_bytes) {
        fprintf(stderr, "Failed to read ROM file: %s\n", rom_path);
        km8_platform_shutdown();
        free(owned_rom_path);
        return EXIT_FAILURE;
    }

    if (rom_size > UINT32_MAX) {
        fprintf(stderr, "ROM file is too large: %s\n", rom_path);
        free(rom_bytes);
        km8_platform_shutdown();
        free(owned_rom_path);
        return EXIT_FAILURE;
    }

    Cartridge* cartridge = km8_load_cartridge(rom_bytes, (uint32_t)rom_size);
    if (!cartridge) {
        fprintf(stderr, "Failed to load ROM (invalid KM8 image?): %s\n", rom_path);
        free(rom_bytes);
        km8_platform_shutdown();
        free(owned_rom_path);
        return EXIT_FAILURE;
    }

    free(rom_bytes);

    Km8PlatformWindowDesc window_desc = {
        .title = "km8",
        .width = KM8_FRAME_WIDTH * KM8_TARGET_SCALE,
        .height = KM8_FRAME_HEIGHT * KM8_TARGET_SCALE,
        .resizable = false
    };

    Km8PlatformWindow* window = km8_platform_window_create(&window_desc);
    if (!window) {
        km8_platform_shutdown();
        free(owned_rom_path);
        return EXIT_FAILURE;
    }

    uint32_t* frame_buffer = (uint32_t*)malloc(KM8_FRAME_WIDTH * KM8_FRAME_HEIGHT * sizeof(uint32_t));
    if (!frame_buffer) {
        km8_platform_window_destroy(window);
        km8_platform_shutdown();
        free(owned_rom_path);
        return EXIT_FAILURE;
    }

    fill_black(frame_buffer, KM8_FRAME_WIDTH, KM8_FRAME_HEIGHT);

    km8_init();
    Km8Context context = km8_create_context();
    context.cartridge = cartridge;
    (void)context;

    uint64_t next_frame_time = km8_get_time_ns();

    while (!km8_platform_window_should_close(window)) {
        km8_platform_poll_events();

        km8_step_cycles(&context, 175104);
        fill_mem_view(&context, frame_buffer, KM8_FRAME_WIDTH, KM8_FRAME_HEIGHT);

        km8_platform_window_present(window,
                                    frame_buffer,
                                    KM8_FRAME_WIDTH * sizeof(uint32_t),
                                    KM8_FRAME_WIDTH,
                                    KM8_FRAME_HEIGHT);
        
        if(context.cpu.state == CPU_HALT) {
            printf("Halted! (%d)\n", context.cpu.halt_code);
            break;
        }

        next_frame_time += KM8_FRAME_INTERVAL_NS;
        uint64_t now = km8_get_time_ns();
        if (next_frame_time > now) {
            km8_sleep_ns(next_frame_time - now);
        } else {
            next_frame_time = now;
        }
    }

    if(context.cpu.state == CPU_HALT) {
        km8_wait_for_any_key();
    }

    free(frame_buffer);
    km8_platform_window_destroy(window);
    km8_platform_shutdown();
    km8_unload_cartridge(cartridge);
    free(owned_rom_path);
    return EXIT_SUCCESS;
}
