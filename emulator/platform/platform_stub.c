#include "platform/platform.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool km8_platform_init(void) { return false; }
void km8_platform_shutdown(void) {}
Km8PlatformWindow* km8_platform_window_create(const Km8PlatformWindowDesc* desc) {
    (void)desc;
    return NULL;
}
void km8_platform_window_destroy(Km8PlatformWindow* window) { (void)window; }
void km8_platform_poll_events(void) {}
bool km8_platform_window_should_close(const Km8PlatformWindow* window) {
    (void)window;
    return true;
}
void km8_platform_window_present(Km8PlatformWindow* window,
                                 const void* pixels,
                                 uint32_t pitch,
                                 uint32_t width,
                                 uint32_t height) {
    (void)window;
    (void)pixels;
    (void)pitch;
    (void)width;
    (void)height;
}

char* km8_platform_open_rom_dialog(void) {
    printf("Enter path to KM8 ROM (.bin): ");
    fflush(stdout);
    char buffer[4096];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return NULL;
    }

    size_t len = strcspn(buffer, "\r\n");
    buffer[len] = '\0';

    if (len == 0) {
        return NULL;
    }

    char* result = (char*)malloc(len + 1);
    if (!result) {
        return NULL;
    }

    memcpy(result, buffer, len + 1);
    return result;
}
