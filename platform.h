#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <intrin.h>

#include "console.h"

#define TILE_SIZE 8

#define FRAMEBUFFER_W 160
#define FRAMEBUFFER_H 120

#define WINDOW_CLIENT_W 640
#define WINDOW_CLIENT_H 480

typedef void(*InitFunc)(void);
typedef void(*UpdateFrameFunc)(float deltaTime);
typedef void(*KeyEventFunc)(uint8_t key, uint8_t state);

typedef struct {
	InitFunc Init;
	UpdateFrameFunc UpdateFrame;
	KeyEventFunc KeyEvent;
} Game;

extern Game* gGame;
extern uint32_t* gFramebuffer;

void PlatformInit(HINSTANCE hInstance, int nCmdShow, Game* game);
void PlatformExit();

uint64_t now_nanos();

#pragma once
