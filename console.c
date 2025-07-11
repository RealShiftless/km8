#include "console.h"
#include "cpu.h"
#include "rom.h"
#include "emulator.h"

void command_out(const char* fmt, ...) {
    printf(" | "); // Prompt or prefix

    char buffer[COMMAND_BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, COMMAND_BUFFER_SIZE, fmt, args);
    va_end(args);

    printf("%s\n", buffer);
    fflush(stdout); // <-- Force output to appear
}

static void CmdPing(char* args[], int argc) {
    if (argc != 1)
    {
        command_out("Warning: expected 0 arguments!");
        return;
    }

    command_out("Pong!");
}

static void CmdPrintOpcodes(char* args[], int argc) {
    if (argc != 1)
    {
        command_out("Warning: expected 0 arguments!");
        return;
    }

    uint8_t opcodeCount = 0;
    for (int i = 0; i < 256; i++)
    {
        if (gOpcodes[i].size == 0)
            continue;

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
        command_out("[0x%02X] %-4.4s %-4.4s %-4.4s", i, gOpcodes[i].mnemonic, operand0, operand1);
    }

    command_out("");
    command_out("Total: %d", opcodeCount);
}

static void CmdLoad(char* args[], int argc) {
    if (argc != 2)
    {
        command_out("Warning: expected 1 arguments! (load <file path>)");
        return;
    }

    uint8_t r = RomLoad(args[1]);

    switch (r)
    {
        case LOAD_SIZEWARN:
            command_out("Warning: rom size mismatch!");
        case LOAD_SUCCESS:
            command_out("Loaded rom!");
            break;

        case LOAD_FAILED:
            command_out("Warning: could not load rom!");
            break;
    }

    gEmulatorFlags |= EMUFLAG_ROMLOADED;
}

static void CmdRomView(char* args[], int argc) {
    if (argc != 2)
    {
        command_out("Warning: expected 1 arguments! (rom_view <address>)");
        return;
    }

    uint16_t value = atoi(args[1]);
    command_out("[0x%02X] %d", value, RomRead(value));
}

static void CmdEmuStart(char* args[], int argc) {
    if (argc != 1)
    {
        command_out("Warning: expected 0 arguments!");
        return;
    }

    if ((gEmulatorFlags & EMUFLAG_ROMLOADED) == 0)
    {
        command_out("Warning: no rom was loaded!");
        return;

    }
    
    command_out("Starting emulation...");

    EmuInit();
    gEmulatorFlags |= EMUFLAG_RUNNING;
}


static void CmdEmuSet(char* args[], int argc)
{
    if (argc != 3)
    {
        command_out("Warning: expected 3 arguments!");
        return;
    }

    char* t = args[1];
    if (strcmp(t, "exec_mode") == 0) {
        if (strcmp(args[2], "normal") == 0) {
            gExecutionMode = EXEC_NORMAL;
            command_out("Set execution mode to normal.");

        }
        else if (strcmp(args[2], "cycle_stepped") == 0) {
            gExecutionMode = EXEC_CYCLE_STEPPED;
            command_out("Set execution mode to cycle stepped.");
        }
        else {
            command_out("Warning: invallid value!");
        }
    }
    else {
        command_out("Warning: invallid name!");
    }
}


static Command gCommandTable[] = {
    { "ping", "Prints pong to the log.", CmdPing },
    { "pr_opcodes", "Prints all opcodes to the log.", CmdPrintOpcodes },
    { "rom_load", "Loads a file into rom. Usage: rom_load <path>.", CmdLoad },
    { "rom_view", "Reads a value from rom at address. Usage: rom_view <address>.", CmdRomView },
    { "emu_start", "Starts the emulation.", CmdEmuStart },
    { "emu_set", "Starts the emulation. Usage: emu_set <name> <value>", CmdEmuSet }
};

int TokenizeCommand(char* command, char* tokens[MAX_TOKENS]) {
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

void ExecuteCommand(char* tokens[], int argc) {
    if (argc == 0) return;

    for (int i = 0; gCommandTable[i].name != NULL; ++i) {
        if (strcmp(tokens[0], gCommandTable[i].name) == 0) {
            gCommandTable[i].func(tokens, argc);
            return;
        }
    }

    command_out("Unknown command: %s", tokens[0]);
}

DWORD WINAPI ConsoleThread(LPVOID param) {
    printf("Retro Console Emulator\n");
    printf("|Version: inDev1\n");
    printf("|Author: shiftless\n");
    printf("|Description: I'm building a custom old school console, first with emulator, then on FPGA and real hardware :)\n");

    char line[COMMAND_BUFFER_SIZE];
    char* tokens[MAX_TOKENS];

    printf("\n > ");
    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        int tokenCount = TokenizeCommand(line, tokens);

        if (strcmp(tokens[0], "help") == 0) {
            for (int i = 0; i < sizeof(gCommandTable) / sizeof(Command); i++)
                command_out("%-16s %s", gCommandTable[i].name, gCommandTable[i].description);

            printf("\n > ");
            continue;
        }

        ExecuteCommand(tokens, tokenCount);

        printf("\n > ");
    }
    return 0;
}