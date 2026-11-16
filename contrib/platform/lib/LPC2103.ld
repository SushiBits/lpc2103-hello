/**
 * @file LPC2103.ld
 *
 * Linker script describing memory layout of LPC2103.
 */

MEMORY {
    IROM (r x) : ORIGIN = 0x00000000, LENGTH = 32k
    IRAM (rw ) : ORIGIN = 0x40000000, LENGTH = 8k
}