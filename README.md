# km8

KM8 is an 8-bit fantasy console implemented as a modular C library with pluggable front-ends. The goal is to deliver a cycle-accurate CPU and bus, fixed-timestep simulation, and a custom toolchain built around the KM8 ISA. The repository also ships a GLFW/OpenGL desktop shell as a reference frontend.

> **Status:** pre-alpha. The cartridge loader and bus scaffolding are present, but large subsystems (CPU, PPU, APU, toolchain) are still under active development.

## Highlights
- Deterministic 60 FPS timing model with a 10.50624 MHz master clock and per-device bus latency.
- Cartridge loader with strict header validation (magic, ROM/RAM bank sizing, flags).
- Extensible bus that dispatches reads/writes through registered `BusDevice` handlers.
- Design notes and ISA documentation maintained in the companion vault (`KM8 Technical Overview.md`, `ISA v1.md`).

## Repository Layout
- `include/` - Public headers (`km8/km8.h`, `km8/cartridge.h`) consumed by embedders.
- `internal/` - Private headers (bus address map, latency definitions, registration helpers).
- `src/` - Core library sources: bus dispatcher, cartridge ROM/ERAM devices, CPU scaffolding.
- `emulator/` - Cross-platform desktop frontend using Win32 or X11 backends with software rendering.
- `roms/` - Placeholder binaries and test ROMs.
- `cmake/` - Package config templates used during installation.

## Building

### Prerequisites
- CMake >= 3.16
- A C99-capable toolchain (MSVC, clang, GCC, etc.)
- On Windows, either MSYS2/MinGW or Visual Studio build tools.

### Configure & Build

```powershell
cmake -S . -B build -G Ninja `
  -DKM8_BUILD_SHARED=ON
cmake --build build
```

To perform an install:

```powershell
cmake --install build --prefix $Env:KM8_PREFIX
```

### Running the Emulator Frontend

After a successful build:

```powershell
.\build\bin\km8.exe path\to\rom.bin
```

The frontend opens a fixed-size window, executes the BIOS handoff, runs the ROM at 60 FPS, and visualises register/WRAM state.

## ROM Header Snapshot

KM8 cartridges begin with a fixed 16-byte header:

| Bytes | Meaning |
| --- | --- |
| 0-2 | ASCII `"KM8"` magic |
| 3 | 0xB0 header tag |
| 4 | ROM bank size code (32 KiB -> 8 MiB) |
| 5 | RAM bank size code (0 -> 64 KiB) |
| 6 | Flags (`bit0`: ERAM, `bit1`: battery, `bit2`: RTC, `bit3`: ROM banking, `bit4`: WRAM banking, `bit5`: 50 Hz region) |
| 7-15 | Cartridge title (ASCII, null padded) |

Payload size must match the declared ROM bank count. When ROM banking is disabled (`flags & 0x08 == 0`), writes to the bank register are ignored. ERAM is only mapped when `flags & 0x01` is set.

## Memory Map & Latencies

| Range | Device | Size | Latency (cycles) | Notes |
| --- | --- | --- | --- | --- |
| 0x0000-0x00FF | BIOS | 256 B | 1 | Boot ROM |
| 0x0100-0x40FF | ROM0 | 16 KiB | 4 | Fixed program ROM |
| 0x4100-0x80FF | ROMn | 16 KiB | 5 | Switchable ROM window |
| 0x8100-0xA0FF | WRAM0 | 8 KiB | 1 | Fast internal RAM |
| 0xA100-0xC0FF | WRAMn | 8 KiB | 2 | Banked work RAM |
| 0xC100-0xE0FF | VRAM | 8 KiB | 2 | Visible writes masked |
| 0xE100-0xF0FF | ERAM | 4 KiB | 8 | External save RAM |
| 0xF100-0xF1FF | OAM | 256 B | 2 | Sprite attribute table |
| 0xF200-0xF2FF | I/O | 256 B | - | Timers, DMA, input, banking |
| 0xF300-0xF3FF | HRAM | 256 B | 1 | Scratchpad |
| 0xF400-0xFFFF | - | 3 KiB | - | Unused / open bus |

## ISA Quick Reference

The KM8 ISA v1 is an 8-register architecture with mixed-width opcodes. A condensed view:

- Load/store (`LDR`, `STR`) operate on 16-bit addresses; immediates are little-endian.
- Arithmetic/logic instructions have register and immediate forms (`ADD`, `SUB`, `AND`, `OR`, `XOR`, `ADC`, `SBC`).
- Flow control includes conditional jumps (`JZ`, `JC`, `JN`, `JV`) in absolute and register-indirect forms, plus `CALL`/`RET` and `HLT`.
- Stack instructions (`PUSH`, `POP`) move whole registers.

See the full opcode table in `ISA v1.md` for exact encodings and reserved slots.

## Developing

1. Initialise the emulator state with `Km8Context ctx = km8_create_context();`
2. Call `km8_bus_init()` to register default devices (BIOS, ROM, ERAM, WRAM). Additional devices can be registered via `km8_bus_device_register`.
3. Drive `km8_step_cycles` to execute CPU cycles; hook the PPU/audio subsystems as they come online.
4. Frontends should run a fixed 60 Hz loop and copy their frame/audio buffers out of the context.

The desktop frontend is intentionally simple so other platforms (SDL, WebGPU, etc.) can mirror the same hooks. Contributions should keep the core library platform-agnostic and push platform specifics under `emulator/`.
