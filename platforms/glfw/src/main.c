#include "window.h"

#include <glad/glad.h>
#include <stdio.h>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (platform_glfw_init()) {
        fprintf(stderr, "Failed to initialize glfw!");
        return 1;
    }

    while (!platform_glfw_should_close()) {
        platform_glfw_poll_events();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        platform_glfw_swap_buffers();
    }

    platform_glfw_shutdown();
    return 0;
}
