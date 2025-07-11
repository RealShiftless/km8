#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define COMMAND_BUFFER_SIZE 256
#define TOKEN_BUFFER_SIZE 64
#define MAX_TOKENS 8

typedef void (*CommandFunc)(char* args[], int argc);

typedef struct {
	const char* name;
	const char* description;
	CommandFunc func;
} Command;

void command_out(const char* fmt, ...);

DWORD WINAPI ConsoleThread(LPVOID param);

#pragma once
