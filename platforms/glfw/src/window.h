#pragma once

#include <stdbool.h>

int platform_glfw_init();
void platform_glfw_shutdown();

bool platform_glfw_should_close();
void platform_glfw_poll_events();
void platform_glfw_swap_buffers();
