#ifndef _LINKAGE_H
#define _LINKAGE_H

#include "types.h"

#ifndef ASM

extern int32_t system_call_handler();

/* Save registers for keyboard handler */
extern void keyboard_linkage();

/* Save registers for rtc handler */
extern void rtc_linkage();

#endif /* ASM */

#endif /* _LINKAGE_H */
