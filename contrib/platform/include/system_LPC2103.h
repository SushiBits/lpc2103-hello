/**
 * @file system_LPC2103.h
 * 
 * LPC2103 System-level Utility
 */

#ifndef _SYSTEM_LPC2103_H_
#define _SYSTEM_LPC2103_H_

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

extern uint32_t SystemCoreClock;
void SystemInit(void);
void SystemCoreClockUpdate(void);

__END_DECLS

#endif
