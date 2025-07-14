#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CLI_INPUT_BUFFER_S 256
#define TOKEN_BUFFER_SIZE 64
#define MAX_TOKENS 8

typedef void (*CommandFunc)(char* args[], int argc);

typedef struct {
	const char* name;
	const char* description;
	CommandFunc func;
} Command;

void cli_lout(const char* fmt, ...);

DWORD WINAPI cli_thread_func(LPVOID param);
void cli_process_input(void);

void cli_exit(void);

void cli_about(void);

void cli_runtime_out(const char* fmt, ...);
void cli_runtime_err_out(const char* fmt, ...);

#pragma once
