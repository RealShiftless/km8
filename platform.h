#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <intrin.h>

#include "console.h"

#define TILE_SIZE 8

#define FRAMEBUFFER_INTERNAL_W 160
#define FRAMEBUFFER_INTERNAL_H 120

#define WINDOW_CLIENT_W 640
#define WINDOW_CLIENT_H 480

typedef void(*InitFunc)(void);
typedef void(*ExitFunc)(void);
typedef void(*UpdateFrameFunc)(float deltaTime);
typedef void(*KeyEventFunc)(uint8_t key, uint8_t state);

typedef struct {
	InitFunc init;
	ExitFunc exit;
	UpdateFrameFunc update_frame;
	KeyEventFunc key_event;
} Game;

extern Game* gGame;
extern uint32_t* gFramebuffer;

void platform_init(HINSTANCE hInstance, int nCmdShow, Game* game);
void platform_exit();

uint64_t now_nanos();

#pragma once
