#include "window.h"

#include <glad/glad.h>
#include <stdio.h>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    WindowErr init_result = km8_window_init();
    if (init_result != KM8_WINDOW_OK) {
        fprintf(stderr, "km8: window init failed: %s\n", km8_window_get_err_string(init_result));
        return 1;
    }

    while (!km8_window_should_close()) {
        km8_window_poll_events();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        km8_window_swap_buffers();
    }

    km8_window_shutdown();
    return 0;
}
