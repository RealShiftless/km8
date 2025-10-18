#pragma once

#include <stdbool.h>

typedef enum {
    KM8_WINDOW_OK = 0,
    KM8_WINDOW_GL_FAILED
} WindowErr;

WindowErr km8_window_init();
void km8_window_shutdown();

bool km8_window_should_close();
void km8_window_poll_events();
void km8_window_swap_buffers();

inline static const char* km8_window_get_err_string(WindowErr err) {
    switch(err) {
        case KM8_WINDOW_OK:             return "Ok!";
        case KM8_WINDOW_GL_FAILED:      return "GL Initialization failed!";

        default:                        return "Unknown error!";
    }
}
