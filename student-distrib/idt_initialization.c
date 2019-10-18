#include "idt_initilization.h"

# define NUM_EXCEPTION 32
# define SYS_CALL_INDEX 0x80

void test_interrupt(void){
	printf("I love Srijan\n");
}


void initialize_idt(void){
	int init_counter;
	
	for(init_counter=0;init_counter<NUM_EXCEPTION;init_counter++){
		
		idt[init_counter].present=1;
		idt[init_counter].dpl=0;
		idt[init_counter].reserved0=0;
		idt[init_counter].size=1;
		idt[init_counter].reserved1=1;
		idt[init_counter].reserved2=1;
		idt[init_counter].reserved3=1;
		idt[init_counter].reserved4 =0;
		idt[init_counter].seg_selector=KERNEL_CS;
		SET_IDT_ENTRY(idt[init_counter],test_interrupt);
	}
	
	for(init_counter=NUM_EXCEPTION;init_counter<NUM_VEC; init_counter++){
		
		
			if(init_counter== SYS_CALL_INDEX){
					continue;
			}
		
			idt[init_counter].present=1;
			idt[init_counter].dpl=0;
			idt[init_counter].reserved0=0;
			idt[init_counter].size=1;
			idt[init_counter].reserved1=1;
			idt[init_counter].reserved2=1;
			idt[init_counter].reserved3=0;
			idt[init_counter].reserved4=0;
			idt[init_counter].seg_selector=KERNEL_CS;
			SET_IDT_ENTRY(idt[init_counter],test_interrupt);
	}
	
	idt[SYS_CALL_INDEX].present=1;
	idt[SYS_CALL_INDEX].dpl=0;
	idt[SYS_CALL_INDEX].reserved0=0;
	idt[SYS_CALL_INDEX].size=1;
	idt[SYS_CALL_INDEX].reserved1=1;
	idt[SYS_CALL_INDEX].reserved2=1;
	idt[SYS_CALL_INDEX].reserved3=0;
	idt[SYS_CALL_INDEX].reserved4=0;
	idt[SYS_CALL_INDEX].seg_selector=KERNEL_CS;
	set_IDT_ENTRY(idt[SYS_CALL_INDEX],test_interrupt);
	
	
	
	
	
}
