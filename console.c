#include "console.h"
#include "cpu.h"
#include "emulator.h"
#include "bus.h"
#include "opcodes.h"

// Values
char gCliInputBuffer[CLI_INPUT_BUFFER_S];
CRITICAL_SECTION gCliMutex;

volatile LONG gShouldExit = 0;

// Function to print the about, during clear and init
void cli_about() {
    printf("KM-8\n");
    printf("|Version: inDev1\n");
    printf("|Author: shiftless\n");
    printf("|Description: I'm building a custom old school console, first with emulator, then on FPGA and real hardware :)\n");
}

// Function used to print in the format of a command output, adds a " | " prefix, and a new line character
void cli_lout(const char* fmt, ...) {
    printf(" | "); // Prompt or prefix

    char buffer[CLI_INPUT_BUFFER_S];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CLI_INPUT_BUFFER_S, fmt, args);
    va_end(args);

    printf("%s\n", buffer);
    fflush(stdout);
}

// Highlighted out used for tables, adds the prefix, then the colored format, without a new line, encapsulated by []
void cli_hout(const char* fmt, ...) {
    printf(" | [");

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 3);

    char buffer[CLI_INPUT_BUFFER_S];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CLI_INPUT_BUFFER_S, fmt, args);
    va_end(args);

    printf("%s", buffer);
    fflush(stdout);

    SetConsoleTextAttribute(hConsole, 0x7);

    printf("] ");
}

// Warn print
void cli_warn(const char* fmt, ...) {
    printf(" | ");

    // Print warning prefix
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    printf("Warning");
    SetConsoleTextAttribute(hConsole, 7);


    char buffer[CLI_INPUT_BUFFER_S];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CLI_INPUT_BUFFER_S, fmt, args);
    va_end(args);

    printf(": %s\n", buffer);
    fflush(stdout);
}

// Reset cursor pos
void cli_reset_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (GetConsoleScreenBufferInfo(hConsole, &info)) {
        COORD cursorPos = { 0, info.dwCursorPosition.Y }; // X = 0 (column), Y = current row
        SetConsoleCursorPosition(hConsole, cursorPos);
    }
}

// Runtime print
void cli_runtime_out(const char* fmt, ...) {
    cli_reset_cursor();
    // Print warning prefix
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    printf("Runtime");
    SetConsoleTextAttribute(hConsole, 7);

    char buffer[CLI_INPUT_BUFFER_S];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CLI_INPUT_BUFFER_S, fmt, args);
    va_end(args);

    printf(": %s\n", buffer);
    fflush(stdout);
}

// Runtime exception print
void cli_runtime_err_out(const char* fmt, ...) {
    cli_reset_cursor();
    // Print warning prefix
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

    printf("Runtime Exception");
    SetConsoleTextAttribute(hConsole, 7);

    char buffer[CLI_INPUT_BUFFER_S];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CLI_INPUT_BUFFER_S, fmt, args);
    va_end(args);

    printf(": %s\n", buffer);
    fflush(stdout);
}

// First test command to test console commands
static void cmd_ping(char* args[], int argc) {
    if (argc != 1)
    {
        cli_warn("Expected 0 arguments!");
        return;
    }

    cli_lout("Pong!");
}

// Prints all opcodes to the console window
static void cmd_pr_opcodes(char* args[], int argc) {
    if (argc != 1)
    {
        cli_warn("Expected 0 arguments!");
        return;
    }

    uint8_t opcodeCount = 0;
    for (int i = 0; i < 256; i++)
    {
        if (gOpcodes[i].size == 0)
            continue;

        // This gives a name to the operand type
        const char* operand0 = "???";
        switch (gOpcodes[i].operands[0].type) {
        case OPERAND_NIL: operand0 = "nil"; break;
        case OPERAND_IMM: operand0 = "imm"; break;
        case OPERAND_REG: operand0 = "reg"; break;
        case OPERAND_MEM: operand0 = "mem"; break;
        }

        const char* operand1 = "???";
        switch (gOpcodes[i].operands[1].type) {
        case OPERAND_NIL: operand1 = "nil"; break;
        case OPERAND_IMM: operand1 = "imm"; break;
        case OPERAND_REG: operand1 = "reg"; break;
        case OPERAND_MEM: operand1 = "mem"; break;
        }

        opcodeCount++;
        cli_hout("0x%02X", i);
        printf("%-4.4s %-4.4s %-4.4s\n", gOpcodes[i].mnemonic, operand0, operand1);
    }

    cli_lout("");
    cli_lout("Total: %d", opcodeCount);
}

// Loads a rom into memory, sets the rom loaded flags
static void cmd_rom_load(char* args[], int argc) {
    if (argc != 2)
    {
        cli_warn("Expected 1 arguments! (load <file path>)");
        return;
    }

    uint8_t r = load_rom(args[1]);

    switch (r)
    {
        case LOAD_FAILED:
            cli_warn("Could not load rom!");
            return;

        case LOAD_INVALLID_HEADER:
            cli_warn("Invallid header!");
            return;

        case LOAD_ALLOC_FAILED:
            cli_warn("Allocation failed!");
            return;
    }

    cli_lout("Loaded rom succesfully!");
    cli_lout("");
    cli_lout("Title: %s", gCartridge.header.game_title);
    cli_lout("Rom size: %dKb", (gCartridge.header.rom_size * CART_ROM_BANK_S) / 1024);
    cli_lout("Ram size: %dKb", (gCartridge.header.ram_size * CART_RAM_BANK_S) / 1024);
}

// View registers
static void cmd_view_regs(char* args[], int argc) {
    if (argc != 1)
    {
        cli_warn("Expected 0 arguments!");
        return;
    }

    for (int i = 0; i < REGISTER_COUNT - 5; i++) {
        uint8_t value = gRegisters[i];

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("r%d", i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }

    cli_hout("sp");
    printf("0x%02X%02X\n", 
        gRegisters[SP_INDEX + 0], 
        gRegisters[SP_INDEX + 1]);

    cli_hout("pc");
    printf("0x%02X%02X\n",
        gRegisters[PC_INDEX + 0],
        gRegisters[PC_INDEX + 1]);
}

// Views in address space
static void cmd_view(char* args[], int argc) {
    if (argc < 2 || argc > 3)
    {
        cli_warn("Expected 1 or 2 arguments! (view <address> [length])");
        return;
    }

    // Get the arguments
    uint32_t address = (uint32_t)strtoul(args[1], NULL, 0);

    uint32_t length = 1;
    if (argc == 3)
        length = (uint32_t)strtoul(args[2], NULL, 0);

    for (int i = 0; i < length; i++) {
        uint8_t value;
        uint8_t latency;
        bus_read(address + i, &value, &latency);

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("0x%04X", address + i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }
}

// Views a address in rom
static void cmd_view_rom(char* args[], int argc) {
    if (gCartridge.rom == NULL)
    {
        cli_warn("No rom was loaded!");
        return;
    }

    if (argc < 2 || argc > 3)
    {
        cli_warn("Expected 1 or 2 arguments! (view_rom <address> [length])");
        return;
    }
    
    // Get the arguments
    uint32_t address = (uint32_t)strtoul(args[1], NULL, 0);

    uint32_t length = 1;
    if (argc == 3)
        length = (uint32_t)strtoul(args[2], NULL, 0);


    for (int i = 0; i < length; i++) {
        uint8_t value = read_rom_flat(address + i);

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("0x%08X", address + i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }
}

// Views addresses in internal ram
static void cmd_view_iram(char* args[], int argc) {
    if (argc < 2 || argc > 3) {
        cli_warn("Expected 1 or 2 arguments! (view_iram <address> [length])");
        return;
    }

    // Get the arguments
    uint32_t address = (uint32_t)strtoul(args[1], NULL, 0);

    uint32_t length = 1;
    if (argc == 3)
        length = (uint32_t)strtoul(args[2], NULL, 0);


    for (int i = 0; i < length; i++) {
        uint8_t value = read_iram_flat(address + i);

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("0x%08X", address + i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }
}

// Views addresses in external ram
static void cmd_view_eram(char* args[], int argc) {
    if (gCartridge.rom == NULL)
    {
        cli_warn("No rom was loaded!");
        return;
    }

    if (argc < 2 || argc > 3)
    {
        cli_warn("Expected 1 or 2 arguments! (view_eram <address> [length])");
        return;
    }

    // Get the arguments
    uint32_t address = (uint32_t)strtoul(args[1], NULL, 0);

    uint32_t length = 1;
    if (argc == 3)
        length = (uint32_t)strtoul(args[2], NULL, 0);

    for (int i = 0; i < length; i++) {
        uint8_t value = read_eram_flat(address + i);

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("0x%08X", address + i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }
}

// Views addresses in high ram
static void cmd_view_hram(char* args[], int argc) {
    if (argc < 2 || argc > 3)
    {
        cli_warn("Expected 1 or 2 arguments! (view_hram <address> [length])");
        return;
    }

    // Get the arguments
    uint8_t address = (uint8_t)strtoul(args[1], NULL, 0);

    uint16_t length = 1;
    if (argc == 3)
        length = (uint16_t)strtoul(args[2], NULL, 0);

    if (address + length > 256)
    {
        cli_warn("Out of range!");
        return;
    }

    for (int i = 0; i < length; i++) {
        uint8_t value = read_hram(address + i);

        char bin[9];
        for (int i = 7; i >= 0; i--)
            bin[7 - i] = ((value >> i) & 1) ? '1' : '0';
        bin[8] = '\0';

        char c = (value >= 32 && value <= 126) ? value : '.';
        cli_hout("0x%08X", address + i);
        printf("0x%02X %3d %s %c\n", value, value, bin, c);
    }
}

// Starts the emulation, if the emu loaded flag is set.
// Also sets the running flag, so the CPU can start running :)
static void cmd_emu_start(char* args[], int argc) {
    if (argc != 1)
    {
        cli_warn("Expected 0 arguments!");
        return;
    }

    if (gCartridge.rom == NULL)
    {
        cli_warn("No rom was loaded!");
        return;

    }
    
    cli_lout("Starting emulation...");

    init_emulation();
    gExecutionMode = EXEC_NORMAL;
    gCpuState = CPU_FETCH_OPCODE;
}

static Command gCommandTable[] = {
    { "ping", "Prints pong to the log.", cmd_ping },
    { "pr_opcodes", "Prints all opcodes to the log.", cmd_pr_opcodes },
    { "rom_load", "Loads a file into rom. Usage: rom_load <path>.", cmd_rom_load },
    { "view", "Reads a value from rom at address range. Usage: view <address> [length].", cmd_view },
    { "view_regs", "Reads all registers.", cmd_view_regs },
    { "view_rom", "Reads a value from rom at address range. Usage: view_rom <address> [length].", cmd_view_rom },
    { "view_iram", "Reads a value from internal memory at address range. Usage: view_iram <address> [length].", cmd_view_iram },
    { "view_eram", "Reads a value from external memory at address range. Usage: view_eram <address> [length].", cmd_view_eram },
    { "view_hram", "Reads a value from external memory at address range. Usage: view_hram <address> [length].", cmd_view_hram },
    { "emu_start", "Starts the emulation.", cmd_emu_start },
};

// Here i tokenize a input string into seperate tokens, where it takes account of "" marked fields
int tokenize_command(char* command, char* tokens[MAX_TOKENS]) {
    uint8_t inString = 0;
    uint8_t tokenCount = 0;

    char* p = command;
    char* tokenStart = NULL;

    while (*p) {
        if (*p == '"') {
            inString = !inString;
            if (inString) {
                tokenStart = p + 1; // skip the quote
            }
            else {
                *p = '\0'; // end the string
                if (tokenCount < MAX_TOKENS) {
                    tokens[tokenCount++] = tokenStart;
                }
                tokenStart = NULL;
            }
        }
        else if ((*p == ' ' || *p == '\t') && !inString) {
            if (tokenStart) {
                *p = '\0'; // terminate token
                if (tokenCount < MAX_TOKENS) {
                    tokens[tokenCount++] = tokenStart;
                }
                tokenStart = NULL;
            }
        }
        else {
            if (!tokenStart) {
                tokenStart = p;
            }
        }
        p++;
    }

    // Final token
    if (tokenStart && tokenCount < MAX_TOKENS) {
        tokens[tokenCount++] = tokenStart;
    }

    return tokenCount;
}

void execute_command(char* tokens[], int argc) {
    if (argc == 0) return;

    for (int i = 0; gCommandTable[i].name != NULL; ++i) {
        if (strcmp(tokens[0], gCommandTable[i].name) == 0) {
            gCommandTable[i].func(tokens, argc);
            return;
        }
    }

    cli_warn("Unknown command %s!", tokens[0]);
}

void cli_process_input() {
    if (gCliInputBuffer[0] == 0)
        return;

    static char* tokens[MAX_TOKENS];
    int tokenCount = tokenize_command(gCliInputBuffer, tokens);

    if (strcmp(tokens[0], "help") == 0) {
        for (int i = 0; i < sizeof(gCommandTable) / sizeof(Command); i++)
            cli_lout("%-16s %s", gCommandTable[i].name, gCommandTable[i].description);

        memset(gCliInputBuffer, 0, CLI_INPUT_BUFFER_S);
        return;
    }

    execute_command(tokens, tokenCount);
    memset(gCliInputBuffer, 0, CLI_INPUT_BUFFER_S);
}

DWORD WINAPI cli_thread_func(LPVOID param) {
    // Do some setup
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    char line[CLI_INPUT_BUFFER_S];

    InitializeCriticalSection(&gCliMutex);

    cli_about();

    // Enter main loop
    while (!gShouldExit)
    {
        EnterCriticalSection(&gCliMutex);
        if (gCliInputBuffer[0] != 0)
        {
            LeaveCriticalSection(&gCliMutex);
            continue;
        }
        printf("\n > ");

        // Read the line
        DWORD read = 0;
        ReadConsoleA(hStdin, line, CLI_INPUT_BUFFER_S, &read, NULL);

        // Null terminate
        line[(read < CLI_INPUT_BUFFER_S) ? read : (CLI_INPUT_BUFFER_S - 1)] = '\0';

        // Trim trailing newline(s)
        while (read > 0 && (line[read - 1] == '\n' || line[read - 1] == '\r'))
            line[--read] = '\0';

        // Critical section to write shared buffer
        
        memcpy(gCliInputBuffer, line, CLI_INPUT_BUFFER_S);
        LeaveCriticalSection(&gCliMutex);

        //printf("\n > "); // Prompt again
    }
    return 0;
}

void cli_exit() {
    DeleteCriticalSection(&gCliMutex);
}

