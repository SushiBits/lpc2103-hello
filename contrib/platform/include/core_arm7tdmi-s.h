/**
 * @file core_arm7tdmi-s.h
 * 
 * ARM7TDMI-S core definitions and functions
 *
 * Filled in from CMSIS Core (Apache-2.0) with everything that ARMv4T does not
 * implement left out rather than present and broken: no WFI, WFE, SEV, BKPT,
 * DMB/DSB/ISB, LDREX/STREX, REV16-as-instruction, CLZ-as-instruction, SSAT/USAT,
 * DSP group, PRIMASK/CONTROL or NVIC. Byte-order and bit helpers are built from
 * operations the core actually has.
 *
 * Note that MRS, MSR, SWP and RRX exist only in ARM state on ARMv4T - Thumb-1
 * has no encoding for them. Translation units that call __get_CPSR, __set_CPSR,
 * __get_mode, __set_mode, __get_SP_usr, __set_SP_usr, __enable_irq,
 * __disable_irq or __RRX must therefore be built without -mthumb. There is no
 * per-function ARM-state attribute in GCC for ARM, so this cannot be localised.
 */

#ifndef _CORE_ARM7TDMI_S_H_
#define _CORE_ARM7TDMI_S_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <inttypes.h>

#define __STATIC_FORCEINLINE __attribute__((always_inline)) static inline
#define __ASM asm

#ifdef __cplusplus
  #define   __I     volatile             /*!< \brief Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< \brief Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< \brief Defines 'write only' permissions */
#define     __IO    volatile             /*!< \brief Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*!< \brief Defines 'read only' structure member permissions */
#define     __OM     volatile            /*!< \brief Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*!< \brief Defines 'read / write' structure member permissions */
#define RESERVED(N, T) T RESERVED##N;    // placeholder struct members used for "reserved" areas

/* Core Register CPSR */
typedef union
{
  struct
  {
    uint32_t M:5;                        /*!< \brief bit:  0.. 4  Mode field */
    uint32_t T:1;                        /*!< \brief bit:      5  Thumb execution state bit */
    uint32_t F:1;                        /*!< \brief bit:      6  FIQ mask bit */
    uint32_t I:1;                        /*!< \brief bit:      7  IRQ mask bit */
    uint32_t A:1;                        /*!< \brief bit:      8  Asynchronous abort mask bit */
    uint32_t E:1;                        /*!< \brief bit:      9  Endianness execution state bit */
    RESERVED(0:6, uint32_t)
    uint32_t GE:4;                       /*!< \brief bit: 16..19  Greater than or Equal flags */
    RESERVED(1:4, uint32_t)
    uint32_t J:1;                        /*!< \brief bit:     24  Jazelle bit */
    RESERVED(2:2, uint32_t)
    uint32_t Q:1;                        /*!< \brief bit:     27  Saturation condition flag */
    uint32_t V:1;                        /*!< \brief bit:     28  Overflow condition code flag */
    uint32_t C:1;                        /*!< \brief bit:     29  Carry condition code flag */
    uint32_t Z:1;                        /*!< \brief bit:     30  Zero condition code flag */
    uint32_t N:1;                        /*!< \brief bit:     31  Negative condition code flag */
  } b;                                   /*!< \brief Structure used for bit  access */
  uint32_t w;                            /*!< \brief Type      used for word access */
} CPSR_Type;

/* CPSR Register Definitions */
#define CPSR_N_Pos                       31U                                    /*!< \brief CPSR: N Position */
#define CPSR_N_Msk                       (1UL << CPSR_N_Pos)                    /*!< \brief CPSR: N Mask */

#define CPSR_Z_Pos                       30U                                    /*!< \brief CPSR: Z Position */
#define CPSR_Z_Msk                       (1UL << CPSR_Z_Pos)                    /*!< \brief CPSR: Z Mask */

#define CPSR_C_Pos                       29U                                    /*!< \brief CPSR: C Position */
#define CPSR_C_Msk                       (1UL << CPSR_C_Pos)                    /*!< \brief CPSR: C Mask */

#define CPSR_V_Pos                       28U                                    /*!< \brief CPSR: V Position */
#define CPSR_V_Msk                       (1UL << CPSR_V_Pos)                    /*!< \brief CPSR: V Mask */

#define CPSR_Q_Pos                       27U                                    /*!< \brief CPSR: Q Position */
#define CPSR_Q_Msk                       (1UL << CPSR_Q_Pos)                    /*!< \brief CPSR: Q Mask */

#define CPSR_J_Pos                       24U                                    /*!< \brief CPSR: J Position */
#define CPSR_J_Msk                       (1UL << CPSR_J_Pos)                    /*!< \brief CPSR: J Mask */

#define CPSR_GE_Pos                      16U                                    /*!< \brief CPSR: GE Position */
#define CPSR_GE_Msk                      (0xFUL << CPSR_GE_Pos)                 /*!< \brief CPSR: GE Mask */

#define CPSR_E_Pos                       9U                                     /*!< \brief CPSR: E Position */
#define CPSR_E_Msk                       (1UL << CPSR_E_Pos)                    /*!< \brief CPSR: E Mask */

#define CPSR_A_Pos                       8U                                     /*!< \brief CPSR: A Position */
#define CPSR_A_Msk                       (1UL << CPSR_A_Pos)                    /*!< \brief CPSR: A Mask */

#define CPSR_I_Pos                       7U                                     /*!< \brief CPSR: I Position */
#define CPSR_I_Msk                       (1UL << CPSR_I_Pos)                    /*!< \brief CPSR: I Mask */

#define CPSR_F_Pos                       6U                                     /*!< \brief CPSR: F Position */
#define CPSR_F_Msk                       (1UL << CPSR_F_Pos)                    /*!< \brief CPSR: F Mask */

#define CPSR_T_Pos                       5U                                     /*!< \brief CPSR: T Position */
#define CPSR_T_Msk                       (1UL << CPSR_T_Pos)                    /*!< \brief CPSR: T Mask */

#define CPSR_M_Pos                       0U                                     /*!< \brief CPSR: M Position */
#define CPSR_M_Msk                       (0x1FUL << CPSR_M_Pos)                 /*!< \brief CPSR: M Mask */

#define CPSR_M_USR                       0x10U                                  /*!< \brief CPSR: M User mode (PL0) */
#define CPSR_M_FIQ                       0x11U                                  /*!< \brief CPSR: M Fast Interrupt mode (PL1) */
#define CPSR_M_IRQ                       0x12U                                  /*!< \brief CPSR: M Interrupt mode (PL1) */
#define CPSR_M_SVC                       0x13U                                  /*!< \brief CPSR: M Supervisor mode (PL1) */
#define CPSR_M_ABT                       0x17U                                  /*!< \brief CPSR: M Abort mode (PL1) */
#define CPSR_M_UND                       0x1BU                                  /*!< \brief CPSR: M Undefined mode (PL1) */
#define CPSR_M_SYS                       0x1FU                                  /*!< \brief CPSR: M System mode (PL1) */

/* ignore some GCC warnings */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
  \brief   No Operation
  \details No Operation does nothing. This instruction can be used for code alignment purposes.
 */
#define __NOP()         __ASM volatile ("nop")

/**
  \brief   Data Synchronization Barrier
  \details ARMv4T has no DMB, DSB or ISB. This core has no data cache and no
           out-of-order execution, and on-chip peripheral writes reach their
           target in program order across the AHB/APB bridges, so the only
           reordering that can bite is the compiler's, which the memory clobber
           prevents. The CP15 forms used on ARM9 and later - c7,c10,4 to drain
           the write buffer and c7,c5,4 to flush the prefetch buffer - are
           deliberately not used: they are outside the ARMv4T base ISA, and an
           undefined CP15 access takes an exception.
 */
__STATIC_FORCEINLINE void __DSB(void)
{
  __ASM volatile ("" ::: "memory");
}

/** \brief  Data Memory Barrier - same stand-in as __DSB on this core.
 */
__STATIC_FORCEINLINE void __DMB(void)
{
  __ASM volatile ("" ::: "memory");
}

/**
  \brief   Instruction Synchronization Barrier
  \details The compiler barrier orders the source; the branch discards the
           prefetched instructions so that fetching resumes after this point,
           which is the whole of instruction serialisation available on a
           three-stage pipeline with no instruction cache.
 */
__STATIC_FORCEINLINE void __ISB(void)
{
  __ASM volatile ("b 1f\n1:" ::: "memory");
}

/** \brief  Compiler-only ordering constraint, emits no code.
 */
#define __COMPILER_BARRIER()   __ASM volatile ("" ::: "memory")

/** \defgroup CMSIS_Core_intrinsics CMSIS Core Intrinsics
  Access to dedicated SIMD instructions
  @{
*/

/** \brief  Get CPSR Register
    \return               CPSR Register value
 */
__STATIC_FORCEINLINE uint32_t __get_CPSR(void)
{
  uint32_t result;
  __ASM volatile("MRS %0, cpsr" : "=r" (result) );
  return(result);
}

/** \brief  Set CPSR Register
    \param [in]    cpsr  CPSR value to set
 */
__STATIC_FORCEINLINE void __set_CPSR(uint32_t cpsr)
{
  __ASM volatile ("MSR cpsr, %0" : : "r" (cpsr) : "cc", "memory");
}

/** \brief  Get Mode
    \return                Processor Mode
 */
__STATIC_FORCEINLINE uint32_t __get_mode(void)
{
  return (__get_CPSR() & 0x1FU);
}

/** \brief  Set Mode
    \param [in]    mode  Mode value to set
 */
__STATIC_FORCEINLINE void __set_mode(uint32_t mode)
{
  __ASM volatile("MSR  cpsr_c, %0" : : "r" (mode) : "memory");
}

/** \brief  Get Stack Pointer
    \return Stack Pointer value
 */
__STATIC_FORCEINLINE uint32_t __get_SP(void)
{
  uint32_t result;
  __ASM volatile("MOV  %0, sp" : "=r" (result) : : "memory");
  return result;
}

/** \brief  Set Stack Pointer
    \param [in]    stack  Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_SP(uint32_t stack)
{
  __ASM volatile("MOV  sp, %0" : : "r" (stack) : "memory");
}

/** \brief  Get USR/SYS Stack Pointer
    \return USR/SYS Stack Pointer value
 */
__STATIC_FORCEINLINE uint32_t __get_SP_usr(void)
{
  uint32_t cpsr = __get_CPSR();
  uint32_t result;
  __ASM volatile(
    "MSR     cpsr_c, %1  \n"
    "MOV     %0, sp   " : "=r"(result) : "r" (CPSR_M_SYS) : "memory"
   );
  __set_CPSR(cpsr);
  __ISB();
  return result;
}

/** \brief  Set USR/SYS Stack Pointer
    \param [in]    topOfProcStack  USR/SYS Stack Pointer value to set
 */
__STATIC_FORCEINLINE void __set_SP_usr(uint32_t topOfProcStack)
{
  uint32_t cpsr = __get_CPSR();
  __ASM volatile(
    "MSR     cpsr_c, %1  \n"
    "MOV     sp, %0   " : : "r" (topOfProcStack), "r" (CPSR_M_SYS) : "memory"
   );
  __set_CPSR(cpsr);
  __ISB();
}

/** \brief  Enable IRQ Interrupts
    \details Clears the CPSR I bit. Only the control field is written, so the
             condition flags and the other mask bits are left alone.
 */
__STATIC_FORCEINLINE void __enable_irq(void)
{
  __ASM volatile ("MSR  cpsr_c, %0" : : "r" (__get_CPSR() & ~CPSR_I_Msk) : "memory");
}

/** \brief  Disable IRQ Interrupts
    \details Sets the CPSR I bit.
 */
__STATIC_FORCEINLINE void __disable_irq(void)
{
  __ASM volatile ("MSR  cpsr_c, %0" : : "r" (__get_CPSR() | CPSR_I_Msk) : "memory");
}

/** \brief  Reverse byte order (32 bit)
    \details 0x12345678 becomes 0x78563412. There is no REV instruction before
             ARMv6, so the compiler synthesises the swap from shifts.
 */
__STATIC_FORCEINLINE uint32_t __REV(uint32_t value)
{
  return __builtin_bswap32(value);
}

/** \brief  Reverse byte order within each halfword
    \details 0x12345678 becomes 0x34127856.
 */
__STATIC_FORCEINLINE uint32_t __REV16(uint32_t value)
{
  return ((uint32_t)__builtin_bswap16((uint16_t)(value >> 16)) << 16) |
         (uint32_t)__builtin_bswap16((uint16_t)value);
}

/** \brief  Reverse byte order and sign-extend a halfword
    \details 0x0080 becomes 0x8000.
 */
__STATIC_FORCEINLINE int16_t __REVSH(int16_t value)
{
  return (int16_t)__builtin_bswap16((uint16_t)value);
}

/** \brief  Rotate Right
    \param [in]    op1  value to rotate
    \param [in]    op2  rotation count, taken modulo 32
 */
__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2)
{
  op2 %= 32U;
  if (op2 == 0U)
  {
    return op1;
  }
  return (op1 >> op2) | (op1 << (32U - op2));
}

/** \brief  Reverse bit order
    \details There is no RBIT instruction before ARMv6; this is the usual
             mask-and-shift exchange, five operations wide.
 */
__STATIC_FORCEINLINE uint32_t __RBIT(uint32_t value)
{
  value = ((value >> 1) & 0x55555555U) | ((value & 0x55555555U) << 1);
  value = ((value >> 2) & 0x33333333U) | ((value & 0x33333333U) << 2);
  value = ((value >> 4) & 0x0F0F0F0FU) | ((value & 0x0F0F0F0FU) << 4);
  value = ((value >> 8) & 0x00FF00FFU) | ((value & 0x00FF00FFU) << 8);
  return (value >> 16) | (value << 16);
}

/** \brief  Rotate Right with Extend
    \details Moves every bit right by one and shifts the carry flag in at the
             left end; the incoming carry is consumed.
 */
__STATIC_FORCEINLINE uint32_t __RRX(uint32_t value)
{
  uint32_t result;
  __ASM ("MOV  %0, %1, rrx" : "=r" (result) : "r" (value));
  return result;
}

/** \brief  Count Leading Zeros
    \details There is no CLZ instruction before ARMv5, so this lowers to
             __clzsi2 from libgcc; the zero case is handled explicitly because
             __builtin_clz(0) is undefined.
 */
__STATIC_FORCEINLINE uint8_t __CLZ(uint32_t value)
{
  if (value == 0U)
  {
    return 32U;
  }
  return (uint8_t)__builtin_clz(value);
}

/* Unaligned access through packed structs, so the compiler emits byte-wise
   loads and stores. ARM7TDMI rotates the result of an unaligned LDR rather than
   faulting, which makes this a correctness fix and not only a strict-aliasing
   one. */

#ifndef __UNALIGNED_UINT16_WRITE
struct T_UINT16_WRITE { uint16_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT16_WRITE(addr, val)  (void)((((struct T_UINT16_WRITE *)(void *)(addr))->v) = (val))
#endif

#ifndef __UNALIGNED_UINT16_READ
struct T_UINT16_READ { uint16_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT16_READ(addr)        (((const struct T_UINT16_READ *)(const void *)(addr))->v)
#endif

#ifndef __UNALIGNED_UINT32_WRITE
struct T_UINT32_WRITE { uint32_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT32_WRITE(addr, val)  (void)((((struct T_UINT32_WRITE *)(void *)(addr))->v) = (val))
#endif

#ifndef __UNALIGNED_UINT32_READ
struct T_UINT32_READ { uint32_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT32_READ(addr)        (((const struct T_UINT32_READ *)(const void *)(addr))->v)
#endif

#ifndef __UNALIGNED_UINT64_WRITE
struct T_UINT64_WRITE { uint64_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT64_WRITE(addr, val)  (void)((((struct T_UINT64_WRITE *)(void *)(addr))->v) = (val))
#endif

#ifndef __UNALIGNED_UINT64_READ
struct T_UINT64_READ { uint64_t v; } __attribute__((packed, aligned(1), __may_alias__));
#define __UNALIGNED_UINT64_READ(addr)        (((const struct T_UINT64_READ *)(const void *)(addr))->v)
#endif

#pragma GCC diagnostic pop

#endif // defined(_CORE_ARM7TDMI_S_H_)