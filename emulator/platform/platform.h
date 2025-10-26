#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Km8PlatformWindow Km8PlatformWindow;

typedef struct Km8PlatformWindowDesc {
    const char* title;
    uint32_t width;
    uint32_t height;
    bool resizable;
} Km8PlatformWindowDesc;

bool km8_platform_init(void);
void km8_platform_shutdown(void);

Km8PlatformWindow* km8_platform_window_create(const Km8PlatformWindowDesc* desc);
void km8_platform_window_destroy(Km8PlatformWindow* window);

void km8_platform_poll_events(void);
bool km8_platform_window_should_close(const Km8PlatformWindow* window);

void km8_platform_window_present(Km8PlatformWindow* window,
                                 const void* pixels,
                                 uint32_t pitch,
                                 uint32_t width,
                                 uint32_t height);

char* km8_platform_open_rom_dialog(void);
