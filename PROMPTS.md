# Prompt and reasoning log

This repository's platform layer was brought up in a long vibe-coding session
with an AI coding agent. This file is the session's paper trail: the prompts as
they were given, what each one produced, and how the disagreements between the
manual and the silicon were settled.

Only the user's messages are reproduced. The agent's operating instructions are
deliberately not part of the record.

## Session metadata

| Field | Value |
| --- | --- |
| Agent | AI assistant using the Copilot SDK in VS Code (`copilot-agent` 1.0.108-0) |
| Model | `deepseek/deepseek-flash` — "DeepSeek V4.1 Flash" |
| Reasoning effort | `high` |
| Capabilities | vision enabled; adaptive thinking unsupported |
| Context window | 1,048,576 tokens (max prompt 655,360, max output 393,216) |
| Working directory | `C:\Users\technix\Documents\Projects\lpc2103-hello` |
| Session id | `a37b8c71-774c-4752-8cd5-990e40552364` |
| Session start | 2026-11-16 20:39:56 UTC (2026-11-17 04:39 +08:00) |
| Subagents | none; the work was done inline |

Values read from the session event log (`session.start`,
`model.model_call_started`), not guessed.

## Before this session

The device description did not start here. Two earlier sessions interrogated the
in-progress SVD — one asking why the UART seemed to have overlapping registers
at the same offsets, one asking why the author was spending hours writing an SVD
by hand at all. By the time this session opened, the SVD covered SYS, VIC, PCB,
GPIO0, FIO0 and UART0/1 and was missing I2C, SPI, SSP, ADC, the timers, the
watchdog and the RTC.

## The prompts

**1. Project import.** "Pace this project import's commits appropriately. This
will be a blinkenlight project. The SVD is as of yet incomplete. Just deal with
Git, don't touch the files."

Eight commits: skeleton and licence, vendor documents, KiCad board, `irqn`
submodule, SVD and schema, linker script, Makefile, editor settings. The SVD
commit body says in so many words that only six peripherals are described so
far, so the incompleteness is history rather than a later surprise.

**2. Re-dating.** "Please re-date the commits appropriately."

Rebuilt the chain with `git commit-tree`, author date equal to committer date,
each commit anchored to the file timestamps it carries (licence Nov 4, manual
Nov 11, board Nov 17 00:35, and so on).

**3. I2C.** "The I2C peripheral is implemented. Please check it into git. (The
state machien is a bastard to write in SVD!)"

Split into the `CONP` field rename and the new `I2C0`/`I2C1` block. Regenerating
the header and reading UM10161 alongside it turned up three defects: the
`SCLH`/`SCLL` duty registers were in the wrong order, the soft/debug interrupts
sat one channel too high, and the `CONP` reset value matched neither of the two
figures the manual gives.

**4. Fixes.** "Thank you for catching the bugs. Please commit them into Git."

One commit per defect, each naming the table it came from.

**5. Silicon as oracle.** "Maybe use the silicon itself as the oracle for CONP?
I have an actual empty LPC2103 available on J-Link. Just make use of my existing
Cortex-Debug configuration and use GDB to attach and query."

Drove `JLinkGDBServerCL.exe` with the same arguments as the launch
configuration and read `PCONP` over GDB. Writing zero, resetting and re-reading
proved the value comes back from reset rather than lingering: **`0x00C8179E`**,
which is exactly the set of bits UM10161's bit table marks as reset 1, and
neither of the two values the manual actually prints. The same session confirmed
`SSPRIS` resets to `0x08` (the register map says `0x04`) and read the whole I2C
block to check the declared reset values.

**6. Progress check-in.** "Check in my current progress to Git in appropriate
cadence."

SPI in full, the register-less SSP placeholder separately. SVDConv's
`M312` address-block errors and a `TIMER1`-named interrupt copy-pasted into
`TIMER2` and `TIMER3` were caught here; the user fixed the names before the
commit.

**7. SSP finished.** "Finally done with SSI. Commit this, and I'll start
tackling ADC and timers. I think the four timer instances are almost bitwise
identical except counter widths?"

Committed the nine SSP registers, then answered the timer question by writing
`0xFFFFFFFF` into `TC`, `PR` and `MR0` of all four timers on the board:
`0xFFFFFFFF` on TIMER0/1, `0x0000FFFF` on TIMER2/3.

**8. Boot checksum tool.** "While I am on commute, create a
contrib/platform/share/lpcsum.py that parses the linked ELF, find the vector
table through its address, calculate and populate the vector table with the
correct checksum for the chip to recognize. Keep in mind that every linker run
can produce different images, so the ELF need to be parsed. Try to make the
resulting file self-contained."

Single-file, standard-library-only tool. UM10161 5.2 defines the checksum: the
reserved vector at `0x14` holds the two's complement of the other seven, so the
eight words sum to zero. Verified against real linker output whose vector table
sat at file offset `0x1000` — the case a hardcoded `0x14` would have destroyed.
Flashing a valid and an invalid image produced no observable difference, which
led to the discovery that J-Link's reset strategy leaves `MEMMAP` in user-flash
mode and never gives the boot loader a vote.

**9. SVD edits, paced.** "Appropriately cadence the edits of the SVD and commit
them."

ADC as one commit, the four timer peripherals as a separate reservation.

**10. Timers finished.** "Timers finished. Pace the commits and submit."

The interrupt-name correction first, the register set second. SVDConv went from
three `M312` errors to zero warnings, and `TIMER_Type` came out at `0x78`.

**11. RTC and WDT.** "Commit my progress so far. Consolidating current time and
alarm time into cross-referencing cluters called DATETIME is DEFINITELY a
cheat."

The clustering is the opposite of a cheat: `NOW` and `ALARM` are sixteen
byte-identical word registers at `0x20` and `0x60`, both read/write, so one
`headerStructName DATETIME` plus `derivedFrom` describes them once and the
generated `RTC_Type` carries the type twice with the gap computed.

**12. SVD complete.** "I think I have everything I need for the SVD. Pace the
commits for me."

The `WDMOD` access fix (set-only does not mean unreadable) split from the RTC
completion, which brought the part to eighteen peripherals and every VIC channel
either named or deliberately reserved.

**13. Core layer, by hand or by theft?** "I think I will need another
file-editing Python script to point "core_other.h" to "core_arm7tdmi.h" where I
will have to put my own intrinsic headers (or can I announce ARM7TDMI as
Cortex-A8 in SVD and steal that part's CMSIS headers?)"

Neither. SVDConv emits the include name for `<cpu><name>other</name>` as
`core_other.h` and has no option to change it, so a two-line shim owns that name
and no post-processing step exists to forget. Declaring an ARMv7-A core on an
ARMv4T part would have made the SVD lie to every other consumer for the sake of
a build niggle.

**14. What is portable from an A-profile header?** "Check for me what can I
steal from this file for ARM7TDMI."

Of `armv7a.h`'s 2180 lines, six: the access qualifiers. The rest is GICv2, CP15
and L1 cache code for silicon this part does not have. Every claim was checked
against `arm-none-eabi-as -march=armv4t` rather than against memory.

**15. Rounding up.** "So rounding up: CPSR-related stuff (includng that struct
expansion of CPSR and IRQ enable and disable); I don't really need the attribute
wrappers as I'll just go direct with GCC attributes; strangely, I can implement
my own `__WFI`, but not in the core file, but the system header, through PCON."

All three hold. The CPSR mode constants need trimming (no Monitor, no
Hypervisor), the attribute wrappers are pointless in a GCC-only tree, and the
`__WFI` equivalent belongs above the core because `wfi` has no ARMv4T encoding —
with the trap that `system_LPC2103.h` is included before the SYS macros exist, so
it has to address PCON directly.

**16. What else is portable?** "What else can I take from this file?"

From `cmsis_gcc.h`: `__ROR`, `__RBIT`, `__RRX`, `__REV`/`__REVSH` (via compiler
builtins), `__CLZ` (via libgcc), `__NOP` and the unaligned accessors. Not
`__WFI`, `__WFE`, `__SEV`, `__BKPT`, the barriers, `__SSAT`/`__USAT`, the
`LDREX`/`STREX` family, `CPSIE`/`CPSID`, or the DSP tail.

**17. Write the core header.** "Take everything you think is useful and put it
into core_arm7tdmi-s.h, and create a sim for ISB and DSB as other copy/pasted
functions use it."

Extended the hand-written header rather than replacing it: barriers, IRQ
masking, byte-order and bit helpers, unaligned accessors. `__DSB`/`__DMB` are
compiler barriers because this core has no data cache, no out-of-order execution
and an ordered write path — the CP15 forms would take an exception if the
hardware does not implement them. `__ISB` adds a taken branch so fetch resumes
after the barrier. Also fixed a live bug: `__get_SP_usr`/`__set_SP_usr` used
`CPS #0x1F`, an ARMv6 instruction that the v4T assembler rejects.

**18. Platform commits.** "Now cadence the staged changes into commits. Finally
I have the platform in alignment with a freshly starting Cortex-M project."

Three commits by layer: core header plus the `.gitignore` rule that makes it
trackable, the system header, and the checksum tool with its Makefile hook and
Python bytecode ignores. The `.gitignore` change was split so each rule landed
with the thing it protects.

**19. The VIC naming clash.** "THe name collision is 100% ARM's fault as NXP
used ARM's VIC IP. Renamed the singular VecAddr to dodge the clash. Commit
this."

Right — ARM's own register map calls both the current-vector register and the
sixteen slot registers `VectAddr`, so SVDConv faithfully generated two members
with the same name and the device header did not compile. The rename to
`CurVectAddr` leaves the slot array under the manual's name; the header then
compiles untouched, with no temporary patching.

**20. This file.** "Create my README and PROMPTS files."

## How the work was checked

Two habits ran through the session and are worth recording, because they are
what the correctness of the SVD rests on:

- **Silicon outranks paper.** Wherever the manual disagreed with itself, a blank
  LPC2103 on a J-Link decided it: write a value, reset, read it back, believe
  the read. More defects turned out to be in the documentation than in the
  transcription.
- **Regenerate, then compile.** Every SVD change was followed by SVDConv, `irqn`
  and an actual `arm-none-eabi-gcc -fsyntax-only` / `-c` build of a translation
  unit that includes the generated headers. That is how the `VectAddr` clash and
  the ARMv6 `CPS` instruction were found — neither is visible by reading.

Commit cadence was deliberately fine-grained: one peripheral or one defect per
commit, with the commit body naming the table the values came from. Nineteen of
the repository's commits touch nothing but the SVD.
