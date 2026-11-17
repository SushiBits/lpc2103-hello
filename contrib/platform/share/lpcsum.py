#!/usr/bin/env python3
"""Patch the LPC2000 boot checksum into a linked ELF image.

The LPC2101/02/03 bootloader hands control to user code only if the reserved
ARM exception vector at 0x00000014 holds the two's complement of the sum of the
other seven vectors, so that the eight words of the vector table sum to zero
(UM10161 5.2, "Criterion for valid user code"). Otherwise it stays in ISP mode
and the application never starts.

The vector table is located through its address, not through a symbol, because
the addresses are what the bootloader looks at and they survive any renaming of
the assembly labels. File offsets are taken from the PT_LOAD program header that
covers that address, so every relink is handled correctly.

This file is deliberately self-contained: standard library only, one file, no
imports from the rest of the tree.
"""

import argparse
import struct
import sys

VECTOR_WORDS = 8
CHECKSUM_INDEX = 5  # the reserved vector at 0x14
VECTOR_BYTES = VECTOR_WORDS * 4
DEFAULT_VECTOR_BASE = 0x00000000

ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2LSB = 1
EM_ARM = 40
PT_LOAD = 1

# ELF32 header and program header layouts, both little-endian.
EHDR = struct.Struct("<16sHHIIIIIHHHHHH")
PHDR = struct.Struct("<IIIIIIII")


class ElfError(Exception):
    """The file is not a usable 32-bit little-endian ARM ELF image."""


class Segment:
    """A PT_LOAD program header, in file and address terms."""

    __slots__ = ("offset", "vaddr", "filesz", "flags")

    def __init__(self, offset, vaddr, filesz, flags):
        self.offset = offset
        self.vaddr = vaddr
        self.filesz = filesz
        self.flags = flags

    def __str__(self):
        perms = "".join(
            letter for letter, bit in (("R", 4), ("W", 2), ("X", 1)) if self.flags & bit
        )
        return "0x%08x..0x%08x -> file 0x%06x (%s)" % (
            self.vaddr,
            self.vaddr + self.filesz,
            self.offset,
            perms or "-",
        )


def load_segments(data):
    """Return the PT_LOAD segments of an ELF image."""
    if len(data) < EHDR.size:
        raise ElfError("file is too short to be an ELF image")
    if data[:4] != ELF_MAGIC:
        raise ElfError("not an ELF file (bad magic)")

    e_ident = data[:16]
    if e_ident[4] != ELFCLASS32:
        raise ElfError("not a 32-bit ELF image (EI_CLASS=%d)" % e_ident[4])
    if e_ident[5] != ELFDATA2LSB:
        raise ElfError("not a little-endian ELF image (EI_DATA=%d)" % e_ident[5])

    fields = EHDR.unpack_from(data, 0)
    e_machine = fields[2]
    e_phoff = fields[5]
    e_phentsize = fields[9]
    e_phnum = fields[10]

    if e_machine != EM_ARM:
        raise ElfError("not an ARM image (e_machine=%d)" % e_machine)
    if e_phentsize != PHDR.size:
        raise ElfError("unexpected program header size %d" % e_phentsize)
    if e_phoff + e_phnum * e_phentsize > len(data):
        raise ElfError("program header table runs past end of file")

    segments = []
    for index in range(e_phnum):
        entry = PHDR.unpack_from(data, e_phoff + index * e_phentsize)
        p_type, p_offset, p_vaddr, _p_paddr, p_filesz, _p_memsz, p_flags, _p_align = entry
        if p_type == PT_LOAD:
            segments.append(Segment(p_offset, p_vaddr, p_filesz, p_flags))
    if not segments:
        raise ElfError("image has no PT_LOAD segments")
    return segments


def find_vector_segment(segments, base):
    """Return the segment covering the whole vector table at *base*, if any."""
    for segment in segments:
        if segment.vaddr <= base and base + VECTOR_BYTES <= segment.vaddr + segment.filesz:
            return segment
    return None


def checksum(words):
    """Two's complement of the sum of the vectors other than the checksum one."""
    total = 0
    for index, word in enumerate(words):
        if index != CHECKSUM_INDEX:
            total += word
    return (-total) & 0xFFFFFFFF


def looks_like_reset_vector(word):
    """True if *word* is a plausible reset vector: a branch or an LDR literal."""
    if (word >> 25) & 0b111 == 0b101:  # B, BL
        return True
    return word & 0x0F7F0000 == 0x051F0000  # LDR PC, [PC, +/-imm]


def process(path, vector_base, dry_run, check, verbose):
    """Checksum one ELF image. Returns a process exit status."""
    with open(path, "rb") as handle:
        data = handle.read()

    segments = load_segments(data)
    segment = find_vector_segment(segments, vector_base)
    if segment is None:
        listing = ", ".join(str(s) for s in segments)
        raise ElfError(
            "no PT_LOAD segment covers the vector table at 0x%08x (loadable: %s);"
            " pass --vector-base if the image is linked elsewhere" % (vector_base, listing)
        )

    file_offset = segment.offset + (vector_base - segment.vaddr)
    words = list(struct.unpack_from("<%dI" % VECTOR_WORDS, data, file_offset))
    new = checksum(words)
    old = words[CHECKSUM_INDEX]

    if verbose:
        for index, word in enumerate(words):
            address = vector_base + index * 4
            label = "checksum" if index == CHECKSUM_INDEX else ""
            print("  %08x  %08x  %s" % (address, word, label))
        if not looks_like_reset_vector(words[0]):
            print(
                "  warning: word at 0x%08x (0x%08x) is not a branch or LDR literal;"
                " the bootloader loads the program counter with 0x%08x"
                % (vector_base, words[0], vector_base),
                file=sys.stderr,
            )

    if check:
        result = (old == new) and (sum(words) & 0xFFFFFFFF) == 0
        print(
            "%s: vector checksum 0x%08x %s"
            % (path, old, "valid" if result else "INVALID (expected 0x%08x)" % new),
            file=sys.stdout if result else sys.stderr,
        )
        return 0 if result else 1

    if old == new:
        print("%s: vector checksum 0x%08x already in place" % (path, new))
        return 0

    if dry_run:
        print("%s: vector checksum would be 0x%08x (currently 0x%08x)" % (path, new, old))
        return 0

    with open(path, "r+b") as handle:
        handle.seek(file_offset + CHECKSUM_INDEX * 4)
        handle.write(struct.pack("<I", new))
    print("%s: vector checksum 0x%08x at 0x%08x (was 0x%08x)" % (path, new, vector_base + 0x14, old))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Write the LPC2000 boot checksum into the vector table of a linked ELF image.",
        epilog="The image is patched in place; re-run objcopy to regenerate ihx/bin output.",
    )
    parser.add_argument("elf", nargs="+", metavar="ELF", help="linked ELF image(s) to patch in place")
    parser.add_argument(
        "-b",
        "--vector-base",
        default=DEFAULT_VECTOR_BASE,
        type=lambda text: int(text, 0),
        help="address of the ARM exception vector table (default: 0x%08x)" % DEFAULT_VECTOR_BASE,
    )
    parser.add_argument("-n", "--dry-run", action="store_true", help="report the checksum without writing")
    parser.add_argument(
        "-c", "--check", action="store_true", help="verify the stored checksum and exit non-zero if wrong"
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="dump the eight vector words")
    args = parser.parse_args(argv)

    status = 0
    for path in args.elf:
        try:
            status |= process(path, args.vector_base, args.dry_run, args.check, args.verbose)
        except (OSError, ElfError) as error:
            print("%s: %s" % (path, error), file=sys.stderr)
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
