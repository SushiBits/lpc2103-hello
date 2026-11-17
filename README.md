# lpc2103-hello

Firmware for the LPC2103 evaluation board in [hardware/](hardware/) — a
blinkenlight, eventually. The board is not the interesting part. The interesting
part is that a 2004-era ARM7TDMI-S is being driven through a project structure
that a freshly started Cortex-M3 project would recognise, built from open
components with exactly one exception.

## A Cortex-M project with an ARM7 inside

The LPC2103 predates CMSIS by roughly half a decade. Its core is ARM7TDMI-S,
which is ARMv4T: no NVIC, no SysTick, no CMSIS vector table, no core header,
and nothing in CMSIS-Core to include. NXP never published a CMSIS-Pack for the
LPC2000 line, and there is no vendor SVD to download — the ones that ever
existed shipped inside Keil MDK installs under a vendor licence.

None of that is hidden here, and the core layer is genuinely hand-written. But
above the core everything is laid out the way modern tooling expects:

| Modern Cortex-M habit | How this project does it |
| --- | --- |
| Device header generated from a CMSIS-SVD | [LPC2103.svd](contrib/platform/share/LPC2103.svd) → SVDConv → `LPC2103.h` |
| Interrupt handler table generated from the IRQn enum | [irqn](contrib/irqn) → `LPC2103_handlers.h` |
| `core_*.h` / `system_*.h` / device header layering | [core_arm7tdmi-s.h](contrib/platform/include/core_arm7tdmi-s.h), [system_LPC2103.h](contrib/platform/include/system_LPC2103.h) |
| Register structs, `*_Pos` / `*_Msk` macros, field enums | generated, straight from the SVD |
| Peripheral view in the debugger during bring-up | cortex-debug loading the same SVD over J-Link |
| Boot image that the chip actually starts | [lpcsum.py](contrib/platform/share/lpcsum.py) writes the vector checksum the boot loader demands |

So the device description, the register map, the interrupt table and the
debugger's peripheral view all come from one machine-readable file, the same way
they would on a Cortex-M part. The only place the 2005 is visible is the core
header: a few hundred lines of qualifiers and intrinsics restricted to what
ARMv4T can actually execute.

## The device description

[LPC2103.svd](contrib/platform/share/LPC2103.svd) is typed in by hand, section
by section, reading UM10161 — no vendor file, no conversion, no scraping. It
took around twenty late-night commits to cover the part, one peripheral per
commit: SYS, VIC, PCB, GPIO0, FIO0, UART0/1, I2C0/1, SPI, SSP, ADC, TIMER0-3,
WDT and RTC. All eighteen are described; every VIC channel the silicon
implements now has a name in the generated `IRQn_Type`.

Because the document is hand-built, the manual was treated as a source to be
checked rather than trusted. Several of its own tables disagree with each other,
and a blank LPC2103 on a J-Link settles those arguments in about a minute:

| Question | Manual | Silicon |
| --- | --- | --- |
| PCONP reset value | `0x03BE` in the register summary, `0x001817BE` in the bit table | **`0x00C8179E`** |
| `SSPRIS` reset value | `0x04` in the register map, `TXRIS` = 1 in the bit table | **`0x08`** |
| Timer 2/3 counter width | 16-bit chapter, but a 32-bit counter paragraph copied in | **16 bits** |

The SVD records the measured values, and the commit messages say where the
number came from. The one thing silicon cannot settle is register *naming* —
which is why the VIC's two `VectAddr` registers had to be disambiguated by hand.

## How the pieces are produced

```
 LPC2103.svd ──SVDConv──▶ LPC2103.h ──irqn──▶ LPC2103_handlers.h
      │                      │                      │
      │                      └── includes ──▶ core_other.h ──▶ core_arm7tdmi-s.h
      │                                      system_LPC2103.h
      │
      └── debugger peripheral view (cortex-debug)

 firmware.elf ──lpcsum.py──▶ checksum patched in, then objcopy ▶ .ihx / .bin
```

`LPC2103.h` and `LPC2103_handlers.h` are generated and therefore ignored by
git; the SVD is the source of truth, and `make headers` rebuilds both. The
hand-written headers are tracked:

- [core_arm7tdmi-s.h](contrib/platform/include/core_arm7tdmi-s.h) — the access
  qualifiers, CPSR view and intrinsics ARMv4T actually has. `MRS`, `MSR`, `SWP`
  and `RRX` exist only in ARM state, so translation units using the CPSR helpers
  must be built without `-mthumb`.
- [core_other.h](contrib/platform/include/core_other.h) — a two-line shim,
  because SVDConv emits `#include "core_other.h"` for `<cpu><name>other</name>`.
  Aliasing the name keeps the generated text untouched, with no post-processing
  step to forget.
- [system_LPC2103.h](contrib/platform/include/system_LPC2103.h) — the system
  layer interface. It is included too early to see the generated SYS macros, so
  anything touching a system register goes through the raw address.
- [lpcsum.py](contrib/platform/share/lpcsum.py) — parses the linked ELF, finds
  the vector table by address through its `PT_LOAD` segment, and writes the
  two's complement checksum the LPC2000 boot loader requires at `0x14`
  (UM10161 5.2). Without it the part stays in ISP and the firmware never runs.

## Requirements

- `arm-none-eabi-gcc` and binutils
- `make`
- `python3`
- Keil's `SVDConv.exe` for regenerating the device header
- `git submodule update --init` for [irqn](contrib/irqn)
- J-Link plus VS Code with cortex-debug, if you want the debugger view

## Status

The platform layer is complete and the SVD covers the whole device; the
firmware itself is not written yet. The [Makefile](Makefile) is carried over
from a Cortex-M0+ project and still needs retargeting to `arm7tdmi` before `make`
produces anything — `-mcpu=cortex-m0plus -mthumb` is not this core.

## Licence

The project is BSD 3-Clause; see [LICENSE.md](LICENSE.md).

Third-party material keeps its own terms:

- **NXP documentation** in [docs/](docs/) is NXP's, redistributed for reference
  and not covered by the project licence. The SVD in this repository is not
  derived from it mechanically; it is a transcription of the register
  descriptions, which is exactly the kind of use the documentation invites.
- **Arm CMSIS** — the structure, naming and intent of
  [core_arm7tdmi-s.h](contrib/platform/include/core_arm7tdmi-s.h) follow CMSIS
  Core, which is Apache-2.0. The bodies are reimplemented for ARMv4T; none of
  the CMSIS headers are vendored.
- **Keil SVDConv** is proprietary and not redistributed here. It is the only
  closed-source component in the toolchain, and it runs from an existing Keil
  installation. Everything else — the compiler, the linker, `make`, Python,
  KiCad, VS Code, cortex-debug — is open source.

## Trademarks

See [TRADEMARKS.md](TRADEMARKS.md).
