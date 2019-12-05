#ifndef _PIT_H
#define _PIT_H

#include "types.h"

#define PIT_IRQ_NUM 0
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND_PORT 0x43


#ifndef ASM

typedef struct sched{
	int32_t process_num
	uint8_t is_running;
	int32_t terminal_num;
}sched_node;

//void enqueue(int32_t num);
//int32_t dequeue(void);

/* initialize the pit */
void pit_init(void);

/* pit interrupt handler */
void pit_interrupt_handler(void);


#endif

#endif
