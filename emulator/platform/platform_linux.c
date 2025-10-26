#if defined(__linux__)

#include "platform/platform.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Km8PlatformWindow {
    Window window;
    GC gc;
    bool should_close;
    uint32_t display_width;
    uint32_t display_height;
    XImage* image;
    uint8_t* image_data;
} Km8PlatformWindow;

static Display* gDisplay = NULL;
static int gScreen = 0;
static Atom gWmDeleteWindow = None;
static Km8PlatformWindow* gActiveWindow = NULL;

static void km8_linux_destroy_image(Km8PlatformWindow* window) {
    if (!window || !window->image) {
        return;
    }

    window->image->data = (char*)window->image_data;
    XDestroyImage(window->image);
    window->image = NULL;
    window->image_data = NULL;
}

bool km8_platform_init(void) {
    if (gDisplay) {
        return true;
    }

    XInitThreads();

    gDisplay = XOpenDisplay(NULL);
    if (!gDisplay) {
        fprintf(stderr, "Failed to open X11 display.\n");
        return false;
    }

    gScreen = DefaultScreen(gDisplay);
    gWmDeleteWindow = XInternAtom(gDisplay, "WM_DELETE_WINDOW", False);

    return true;
}

void km8_platform_shutdown(void) {
    if (gDisplay) {
        XCloseDisplay(gDisplay);
        gDisplay = NULL;
        gActiveWindow = NULL;
        gWmDeleteWindow = None;
    }
}

Km8PlatformWindow* km8_platform_window_create(const Km8PlatformWindowDesc* desc) {
    if (!gDisplay || !desc) {
        return NULL;
    }

    Km8PlatformWindow* window = (Km8PlatformWindow*)calloc(1, sizeof(Km8PlatformWindow));
    if (!window) {
        return NULL;
    }

    uint32_t width = desc->width ? desc->width : 640;
    uint32_t height = desc->height ? desc->height : 480;

    window->display_width = width;
    window->display_height = height;

    Window root = RootWindow(gDisplay, gScreen);

    XSetWindowAttributes attrs;
    memset(&attrs, 0, sizeof(attrs));
    attrs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
    attrs.background_pixel = BlackPixel(gDisplay, gScreen);

    window->window = XCreateWindow(
        gDisplay,
        root,
        0,
        0,
        width,
        height,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask | CWBackPixel,
        &attrs);

    if (!window->window) {
        free(window);
        return NULL;
    }

    window->gc = XCreateGC(gDisplay, window->window, 0, NULL);
    if (!window->gc) {
        XDestroyWindow(gDisplay, window->window);
        free(window);
        return NULL;
    }

    if (desc->title) {
        XStoreName(gDisplay, window->window, desc->title);
    }

    XSetWMProtocols(gDisplay, window->window, &gWmDeleteWindow, 1);

    XMapWindow(gDisplay, window->window);
    XFlush(gDisplay);

    gActiveWindow = window;

    return window;
}

void km8_platform_window_destroy(Km8PlatformWindow* window) {
    if (!window || !gDisplay) {
        return;
    }

    km8_linux_destroy_image(window);

    if (window->gc) {
        XFreeGC(gDisplay, window->gc);
        window->gc = 0;
    }

    if (window->window) {
        XDestroyWindow(gDisplay, window->window);
        window->window = 0;
    }

    if (gActiveWindow == window) {
        gActiveWindow = NULL;
    }

    free(window);
}

void km8_platform_poll_events(void) {
    if (!gDisplay) {
        return;
    }

    while (XPending(gDisplay)) {
        XEvent event;
        XNextEvent(gDisplay, &event);

        if (!gActiveWindow) {
            continue;
        }

        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == gWmDeleteWindow) {
                    gActiveWindow->should_close = true;
                }
                break;

            case DestroyNotify:
                if (gActiveWindow && event.xdestroywindow.window == gActiveWindow->window) {
                    gActiveWindow->should_close = true;
                }
                break;

            case ConfigureNotify:
                gActiveWindow->display_width = (uint32_t)event.xconfigure.width;
                gActiveWindow->display_height = (uint32_t)event.xconfigure.height;
                break;

            case Expose:
                if (gActiveWindow->image) {
                    XPutImage(gDisplay,
                              gActiveWindow->window,
                              gActiveWindow->gc,
                              gActiveWindow->image,
                              0,
                              0,
                              0,
                              0,
                              gActiveWindow->image->width,
                              gActiveWindow->image->height);
                }
                break;

            default:
                break;
        }
    }
}

bool km8_platform_window_should_close(const Km8PlatformWindow* window) {
    return window ? window->should_close : true;
}

void km8_platform_window_present(Km8PlatformWindow* window,
                                 const void* pixels,
                                 uint32_t pitch,
                                 uint32_t width,
                                 uint32_t height) {
    if (!window || !gDisplay || !pixels || width == 0 || height == 0) {
        return;
    }

    (void)pitch; /* assume tightly packed input */

    uint32_t dest_width = window->display_width;
    uint32_t dest_height = window->display_height;

    if (dest_width == 0 || dest_height == 0) {
        return;
    }

    if (!window->image ||
        window->image->width != (int)dest_width ||
        window->image->height != (int)dest_height) {
        km8_linux_destroy_image(window);

        size_t data_size = (size_t)dest_width * dest_height * 4;
        window->image_data = (uint8_t*)malloc(data_size);
        if (!window->image_data) {
            return;
        }

        window->image = XCreateImage(
            gDisplay,
            DefaultVisual(gDisplay, gScreen),
            24,
            ZPixmap,
            0,
            (char*)window->image_data,
            dest_width,
            dest_height,
            32,
            dest_width * 4);

        if (!window->image) {
            free(window->image_data);
            window->image_data = NULL;
            return;
        }

        window->image->byte_order = LSBFirst;
        window->image->bitmap_bit_order = LSBFirst;
    }

    const uint32_t* src = (const uint32_t*)pixels;
    uint8_t* dest = window->image_data;

    for (uint32_t y = 0; y < dest_height; ++y) {
        uint32_t src_y = (uint32_t)(((uint64_t)y * height) / dest_height);
        if (src_y >= height) {
            src_y = height - 1;
        }

        for (uint32_t x = 0; x < dest_width; ++x) {
            uint32_t src_x = (uint32_t)(((uint64_t)x * width) / dest_width);
            if (src_x >= width) {
                src_x = width - 1;
            }

            uint32_t src_pixel = src[src_y * width + src_x];
            uint8_t r = (uint8_t)(src_pixel & 0xFF);
            uint8_t g = (uint8_t)((src_pixel >> 8) & 0xFF);
            uint8_t b = (uint8_t)((src_pixel >> 16) & 0xFF);

            size_t dest_index = ((size_t)y * dest_width + x) * 4;
            dest[dest_index + 0] = b;
            dest[dest_index + 1] = g;
            dest[dest_index + 2] = r;
            dest[dest_index + 3] = 0x00;
        }
    }

    XPutImage(gDisplay,
              window->window,
              window->gc,
              window->image,
              0,
              0,
              0,
              0,
              window->image->width,
              window->image->height);

    XFlush(gDisplay);
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

#endif /* __linux__ */
