#include "lib.h"
#include "pit.h"
#include "i8259.h"

#define OSCILLATOR_FREQ 1193182      /* PIT oscillator runs at approximately 1.193182 MHz */
#define INTERRUPT_INTERVAL 100    /* we want PIT interrupts every 100Hz = 10ms */
#define SCHED_SIZE 3
int32_t sched_arr[3];
int32_t count=0;
/*
int32_t dequeue(void){
	int32_t current_num= sched_arr[head];
	head=(head+1)%SCHED_SIZE;
	return current_num;
}

void enqueue(int32_t num){
	sched_arr[tail]=num;
	tail=(tail+1)%SCHED_SIZE;
}
*/

void pit_init(void){
  uint32_t divisor= OSCILLATOR_FREQ / INTERRUPT_INTERVAL; /* frequency divisor of PIT */
  uint32_t divisor_low = divisor & 0xFF; /* low byte of divisor */
  uint32_t divisor_high = (divisor>>8) & 0xFF; /* high byte of divisor */
  
  sched_arr[0]=-1;
  sched_arr[1]=-1;
  sched_arr[2]=-1;
  

  outb(0x36,PIT_COMMAND_PORT); /* 0x36 = command to set PIT to repeating mode */
  outb(divisor_low,PIT_CHANNEL0); /* write low and high byte of divisor to channel 0 */
  outb(divisor_high,PIT_CHANNEL0);

  /* Enable PIT interrupts on PIC */
  enable_irq(PIT_IRQ_NUM);
}

void pit_interrupt_handler(void){
  unsigned long flags; /* Hold the current flags */

  /* Mask interrupt flags */
  cli_and_save(flags);

  /* Turn off PIC interrupts */
  disable_irq(PIT_IRQ_NUM);

  /* Send EOI signal */
  send_eoi(PIT_IRQ_NUM);

  // printf("PIT > RTC dont @ me\n");

  /* Re-enable interrupts and restores flags */
  
  count=(count+1)%SCHED_SIZE
  while(sched_arr[count]==-1){
	  count= (count+1)%SCHED_SIZE;
  }
  process_switch(sched_arr[count]);
  
  restore_flags(flags);

  /* Unmask PIC interrupts*/
  enable_irq(PIT_IRQ_NUM);
}
