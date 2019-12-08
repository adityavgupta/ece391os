#include "lib.h"
#include "pit.h"
#include "i8259.h"
#include "syscalls.h"
#include "paging.h"
#include "x86_desc.h"

#define OSCILLATOR_FREQ 1193182      /* PIT oscillator runs at approximately 1.193182 MHz */
#define INTERRUPT_INTERVAL 100    /* we want PIT interrupts every 100Hz = 10ms */

int32_t cur_sched_term = 0;
int32_t cur_terminal = 0;
sched_node sched_arr[SCHED_SIZE];

void init_sched(){
	sched_arr[0].process_num = 1;
  sched_arr[0].terminal_num = 0;
  sched_arr[0].video_buffer = FIRST_SHELL;
  sched_arr[0].esp = EIGHT_MB;
  sched_arr[0].ebp = EIGHT_MB;

  sched_arr[1].process_num = -1;
  sched_arr[1].terminal_num = -1;
  sched_arr[1].video_buffer = 0;
  sched_arr[1].esp = 0;
  sched_arr[1].ebp = 0;

  sched_arr[2].process_num = -1;
  sched_arr[2].terminal_num = -1;
  sched_arr[2].video_buffer = 0;
  sched_arr[2].esp = 0;
  sched_arr[2].ebp = 0;
}

void pit_init(void){
  uint32_t divisor = OSCILLATOR_FREQ / INTERRUPT_INTERVAL; /* frequency divisor of PIT */
  uint32_t divisor_low = divisor & 0xFF; /* low byte of divisor */
  uint32_t divisor_high = (divisor >> 8) & 0xFF; /* high byte of divisor */

	init_sched();

  outb(0x36, PIT_COMMAND_PORT); /* 0x36 = command to set PIT to repeating mode */
  outb(divisor_low, PIT_CHANNEL0); /* write low and high byte of divisor to channel 0 */
  outb(divisor_high, PIT_CHANNEL0);

  /* Enable PIT interrupts on PIC */
  enable_irq(PIT_IRQ_NUM);
}

//next_proc = index into sched_arr
void switch_process(int32_t cur_proc, int32_t next_proc){
  // pcb_t* cur_pcb = get_pcb_add();

  if(cur_proc == next_proc) return;


  tss.esp0 = EIGHT_MB - (sched_arr[next_proc].process_num - 1)*EIGHT_KB;
  // tss.esp0 = sched_arr[next_proc].esp;

  set_page_dir_entry(USER_PROG, EIGHT_MB + (sched_arr[next_proc].process_num - 1)*FOUR_MB);

  if(sched_arr[next_proc].terminal_num == cur_terminal){
    set_page_table1_entry(VIDEO_MEM_ADDR, VIDEO_MEM_ADDR);
  } else{
    set_page_table1_entry(VIDEO_MEM_ADDR, sched_arr[next_proc].video_buffer);
  }

	asm volatile("    \n\
		 movl %%ebp, %%eax \n\
		 movl %%esp, %%ebx"
		 :
		 :
		 : "eax","ebx"
	);

  asm volatile("    \n\
     movl %0, %%esp \n\
     movl %1,%%ebp"
     :
     : "r"(sched_arr[next_proc].esp), "r"(sched_arr[next_proc].ebp)
  );

  asm volatile ("      \n\
     movl %%cr3, %%eax \n\
     movl %%eax, %%cr3"
     :
     :
     : "eax"
	);
}

void pit_interrupt_handler(void){
  unsigned long flags; /* Hold the current flags */

  /* Mask interrupt flags */
  cli_and_save(flags);

  /* Turn off PIC interrupts */
  disable_irq(PIT_IRQ_NUM);

  /* Send EOI signal */
  send_eoi(PIT_IRQ_NUM);

	int32_t cur_proc = cur_sched_term;

  cur_sched_term = (cur_sched_term + 1) % SCHED_SIZE;

  int32_t init_sched_term = cur_sched_term;

  while(sched_arr[cur_sched_term].process_num == -1){
    cur_sched_term = (cur_sched_term + 1) % SCHED_SIZE;
    if(cur_sched_term == init_sched_term){
			cur_sched_term=0;
      /* Re-enable interrupts and restores flags */
      restore_flags(flags);
      /* Unmask PIC interrupts*/
      enable_irq(PIT_IRQ_NUM);
      return;
    }
  }

  switch_process(cur_proc,cur_sched_term);

  /* Re-enable interrupts and restores flags */
  restore_flags(flags);

  /* Unmask PIC interrupts*/
  enable_irq(PIT_IRQ_NUM);
}
