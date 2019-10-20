#ifndef _LINKAGE_H
#define _LINKAGE_H

#ifndef ASM

/* Save registers for keyboard handler */
extern void keyboard_linkage();

/* Save registers for rtc handler */
extern void rtc_linkage();

#endif
