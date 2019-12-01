#ifndef _PIT_H
#define _PIT_H

#include "types.h"

#define PIT_IRQ_NUM 0
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND_PORT 0x43


#ifndef ASM

/* initialize the pit */
void pit_init(void);

/* pit interrupt handler */
void pit_interrupt_handler(void);


#endif

#endif
