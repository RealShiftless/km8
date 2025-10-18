#include "window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

GLFWwindow *gWindow;

WindowErr km8_window_init() {
    if (!glfwInit()) {
        return KM8_WINDOW_GL_FAILED;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "km8", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return KM8_WINDOW_GL_FAILED;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return KM8_WINDOW_GL_FAILED;
    }

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    gWindow = window;
    return KM8_WINDOW_OK;
}

void km8_window_shutdown() {
    if (gWindow) {
        glfwDestroyWindow(gWindow);
        gWindow = NULL;
    }
    glfwTerminate();
}

bool km8_window_should_close() {
    return gWindow == NULL || glfwWindowShouldClose(gWindow);
}

void km8_window_poll_events() {
    glfwPollEvents();
}

void km8_window_swap_buffers() {
    if (!gWindow) {
        return;
    }
    glfwSwapBuffers(gWindow);
}
