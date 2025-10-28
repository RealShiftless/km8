# Details

Date : 2025-10-27 23:20:19

Directory d:\\My Folders\\Programming\\C\\km8

Total : 55 files,  3390 codes, 128 comments, 808 blanks, all 4326 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [CMakeLists.txt](/CMakeLists.txt) | CMake | 84 | 0 | 16 | 100 |
| [README.md](/README.md) | Markdown | 74 | 0 | 28 | 102 |
| [emulator/CMakeLists.txt](/emulator/CMakeLists.txt) | CMake | 27 | 0 | 6 | 33 |
| [emulator/main.c](/emulator/main.c) | C | 230 | 0 | 48 | 278 |
| [emulator/platform/platform.h](/emulator/platform/platform.h) | C++ | 22 | 0 | 9 | 31 |
| [emulator/platform/platform\_linux.c](/emulator/platform/platform_linux.c) | C | 264 | 0 | 66 | 330 |
| [emulator/platform/platform\_stub.c](/emulator/platform/platform_stub.c) | C | 47 | 0 | 8 | 55 |
| [emulator/platform/platform\_win32.c](/emulator/platform/platform_win32.c) | C | 189 | 0 | 45 | 234 |
| [include/km8/cartridge.h](/include/km8/cartridge.h) | C | 17 | 2 | 11 | 30 |
| [include/km8/cpu.h](/include/km8/cpu.h) | C | 69 | 0 | 21 | 90 |
| [include/km8/km8.h](/include/km8/km8.h) | C | 31 | 0 | 11 | 42 |
| [include/km8/rom.h](/include/km8/rom.h) | C++ | 0 | 0 | 1 | 1 |
| [internal/bus.h](/internal/bus.h) | C++ | 69 | 24 | 27 | 120 |
| [internal/cpu\_internal.h](/internal/cpu_internal.h) | C | 39 | 2 | 12 | 53 |
| [internal/km8\_internal.h](/internal/km8_internal.h) | C++ | 3 | 0 | 4 | 7 |
| [roms/inc\_loop.kasm](/roms/inc_loop.kasm) | KASM | 5 | 0 | 0 | 5 |
| [src/bus/bus.c](/src/bus/bus.c) | C | 41 | 0 | 13 | 54 |
| [src/bus/dispatch.c](/src/bus/dispatch.c) | C | 15 | 0 | 8 | 23 |
| [src/cartridge/cartridge.c](/src/cartridge/cartridge.c) | C | 84 | 9 | 22 | 115 |
| [src/cartridge/ram.c](/src/cartridge/ram.c) | C | 33 | 4 | 13 | 50 |
| [src/cartridge/rom.c](/src/cartridge/rom.c) | C | 33 | 3 | 11 | 47 |
| [src/core.c](/src/core.c) | C | 20 | 2 | 7 | 29 |
| [src/cpu/alu.c](/src/cpu/alu.c) | C | 73 | 0 | 31 | 104 |
| [src/cpu/cpu.c](/src/cpu/cpu.c) | C | 95 | 3 | 30 | 128 |
| [src/cpu/opcodes.c](/src/cpu/opcodes.c) | C | 587 | 18 | 91 | 696 |
| [src/mem/bios.c](/src/mem/bios.c) | C | 24 | 6 | 10 | 40 |
| [src/mem/wram.c](/src/mem/wram.c) | C | 37 | 0 | 13 | 50 |
| [tools/kasm/AssemblerContext.cs](/tools/kasm/AssemblerContext.cs) | C# | 29 | 3 | 11 | 43 |
| [tools/kasm/Exceptions/KasmInternalException.cs](/tools/kasm/Exceptions/KasmInternalException.cs) | C# | 12 | 0 | 2 | 14 |
| [tools/kasm/InstructionSet.cs](/tools/kasm/InstructionSet.cs) | C# | 37 | 1 | 11 | 49 |
| [tools/kasm/Parsing/Exceptions/KasmSyntaxException.cs](/tools/kasm/Parsing/Exceptions/KasmSyntaxException.cs) | C# | 14 | 0 | 3 | 17 |
| [tools/kasm/Parsing/Parser.cs](/tools/kasm/Parsing/Parser.cs) | C# | 58 | 0 | 16 | 74 |
| [tools/kasm/Parsing/Statement.cs](/tools/kasm/Parsing/Statement.cs) | C# | 36 | 0 | 7 | 43 |
| [tools/kasm/Parsing/StatementBuilder.cs](/tools/kasm/Parsing/StatementBuilder.cs) | C# | 27 | 3 | 10 | 40 |
| [tools/kasm/Parsing/StatementSequence.cs](/tools/kasm/Parsing/StatementSequence.cs) | C# | 18 | 3 | 6 | 27 |
| [tools/kasm/Parsing/StatementSequenceBuilder.cs](/tools/kasm/Parsing/StatementSequenceBuilder.cs) | C# | 27 | 3 | 10 | 40 |
| [tools/kasm/Program.cs](/tools/kasm/Program.cs) | C# | 106 | 9 | 36 | 151 |
| [tools/kasm/Symbols/Symbol.cs](/tools/kasm/Symbols/Symbol.cs) | C# | 20 | 0 | 3 | 23 |
| [tools/kasm/Symbols/SymbolTable.cs](/tools/kasm/Symbols/SymbolTable.cs) | C# | 20 | 3 | 7 | 30 |
| [tools/kasm/Tokenization/ITokenHandler.cs](/tools/kasm/Tokenization/ITokenHandler.cs) | C# | 19 | 0 | 5 | 24 |
| [tools/kasm/Tokenization/Token.cs](/tools/kasm/Tokenization/Token.cs) | C# | 19 | 0 | 3 | 22 |
| [tools/kasm/Tokenization/TokenSequence.cs](/tools/kasm/Tokenization/TokenSequence.cs) | C# | 18 | 3 | 6 | 27 |
| [tools/kasm/Tokenization/TokenSequenceBuilder.cs](/tools/kasm/Tokenization/TokenSequenceBuilder.cs) | C# | 24 | 0 | 5 | 29 |
| [tools/kasm/Tokenization/TokenTypes/AddressLiteralTokenHandler.cs](/tools/kasm/Tokenization/TokenTypes/AddressLiteralTokenHandler.cs) | C# | 31 | 4 | 12 | 47 |
| [tools/kasm/Tokenization/TokenTypes/DirectiveTokenHandler.cs](/tools/kasm/Tokenization/TokenTypes/DirectiveTokenHandler.cs) | C# | 16 | 4 | 11 | 31 |
| [tools/kasm/Tokenization/TokenTypes/EOFToken.cs](/tools/kasm/Tokenization/TokenTypes/EOFToken.cs) | C# | 18 | 0 | 4 | 22 |
| [tools/kasm/Tokenization/TokenTypes/IdentifierToken.cs](/tools/kasm/Tokenization/TokenTypes/IdentifierToken.cs) | C# | 16 | 4 | 13 | 33 |
| [tools/kasm/Tokenization/TokenTypes/InstructionToken.cs](/tools/kasm/Tokenization/TokenTypes/InstructionToken.cs) | C# | 17 | 3 | 7 | 27 |
| [tools/kasm/Tokenization/TokenTypes/IntegerLiteralToken.cs](/tools/kasm/Tokenization/TokenTypes/IntegerLiteralToken.cs) | C# | 31 | 4 | 12 | 47 |
| [tools/kasm/Tokenization/TokenTypes/LabelToken.cs](/tools/kasm/Tokenization/TokenTypes/LabelToken.cs) | C# | 25 | 4 | 10 | 39 |
| [tools/kasm/Tokenization/TokenTypes/NewLineToken.cs](/tools/kasm/Tokenization/TokenTypes/NewLineToken.cs) | C# | 20 | 0 | 5 | 25 |
| [tools/kasm/Tokenization/TokenTypes/RegisterToken.cs](/tools/kasm/Tokenization/TokenTypes/RegisterToken.cs) | C# | 34 | 4 | 11 | 49 |
| [tools/kasm/Tokenization/Tokenizer.cs](/tools/kasm/Tokenization/Tokenizer.cs) | C# | 54 | 0 | 20 | 74 |
| [tools/kasm/kasm.csproj](/tools/kasm/kasm.csproj) | XML | 17 | 0 | 0 | 17 |
| [tools/kasm/km8-isa-latest.json](/tools/kasm/km8-isa-latest.json) | JSON | 415 | 0 | 0 | 415 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)