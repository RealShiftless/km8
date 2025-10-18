#include "window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

GLFWwindow *gWindow;

int platform_glfw_init() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "km8", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    gWindow = window;
    return 0;
}

void platform_glfw_shutdown() {
    if (gWindow) {
        glfwDestroyWindow(gWindow);
        gWindow = NULL;
    }
    glfwTerminate();
}

bool platform_glfw_should_close() {
    return gWindow == NULL || glfwWindowShouldClose(gWindow);
}

void platform_glfw_poll_events() {
    glfwPollEvents();
}

void platform_glfw_swap_buffers() {
    if (!gWindow) {
        return;
    }
    glfwSwapBuffers(gWindow);
}
