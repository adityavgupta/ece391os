#include "idt_init.h"
#include "x86_desc.h"
#include "rtc.h"
#include "lib.h"
#include "i8259.h"

# define NUM_EXCEPTION 32
# define SYS_CALL_INDEX 0x80

void irq0_handler(void){
  send_eoi(0);
  printf("timer chip interrupt\n");
}

void irq1_handler(void){
  send_eoi(1);
  printf("keyboard interrupt\n");
}

void irq2_handler(void){
  send_eoi(2);
  printf("slave interrupt\n");
}

void irq3_handler(void){
  send_eoi(3);
  printf("irq3 interrupt\n");
}

void irq4_handler(void){
  send_eoi(4);
  printf("serial port interrupt\n");
}

void irq5_handler(void){
  send_eoi(5);
  printf("irq_5\n");
}

void irq6_handler(void){
  send_eoi(6);
  printf("irq_6\n");
}

void irq7_handler(void){
  send_eoi(7);
  printf("irq_7");
}

void irq9_handler(void){
  send_eoi(9);
  printf("irq_9\n");
}

void irq10_handler(void){
  send_eoi(10);
  printf("irq_10\n");
}

void irq11_handler(void){
  send_eoi(11);
  printf("irq_11, eth0(network)\n");
}

void irq12_handler(void){
  send_eoi(12);
  printf("irq_12, PS/2 mouse interrupt\n");
}

void irq13_handler(void){
  send_eoi(13);
  printf("irq_13\n");
}

void irq14_handler(void){
  send_eoi(14);
  printf("irq_14, ide0(hard drive interrupt)\n");
}

void irq15_handler(void){
  send_eoi(15);
  printf("irq_15\n");
}

void sys_call_handler(void){
	printf("System call handler here!\n");
}

//Description- Random test function
//INPUTS-None
//Outputs-None
//Return Value- none
//Side Effects- prints the string "Steven Lumetta is an AI coded by Steven Lumetta" to the screen

void test_interrupt(void){
	printf("Steve Lumetta is an AI coded by Steven Lumetta\n");
}


#define EXCEPTION_MAKER(x,msg) \
	void x(void){ \
		printf("Exception: %s",msg); \
        while(2);\
     }

EXCEPTION_MAKER(DIVIDE_ERROR,"Divide Error");
EXCEPTION_MAKER(RESERVED,"RESERVED");
EXCEPTION_MAKER(NMI,"NMI"); //I know this is probably wrong I will need to change in the future
EXCEPTION_MAKER(BREAKPOINT,"Breakpoint");
EXCEPTION_MAKER(OVERFLOW,"Overflow");
EXCEPTION_MAKER(BOUND_RANGE_EXCEEDED,"BOUND Range Exceeded");
EXCEPTION_MAKER(INVALID_OPCODE,"Invalid OPCODE");
EXCEPTION_MAKER(DEVICE_NOT,"Device Not Available");
EXCEPTION_MAKER(DOUBLE_FAULT,"Double Fault");
EXCEPTION_MAKER(SEGMENT_OVERRUN,"Coprocessor Segment Overrun");
EXCEPTION_MAKER(INVALID_TSS,"Invalid TSS");
EXCEPTION_MAKER(SEGMENT_NOT_PRESENT,"Segment Not Present");
EXCEPTION_MAKER(STACK_SEGMENT_FAULT,"Stack Segment Fault") ;
EXCEPTION_MAKER(GENERAL_PROTECTION, "General Protection");
EXCEPTION_MAKER(PAGE_FAULT, "Page Fault");
//EXCEPTION_MAKER(Intel_reserved,"Intel Reserved. Do not use");
EXCEPTION_MAKER(MATH_FAULT,"x87 FPU Floating-Point Error(Math Fault)");
EXCEPTION_MAKER(ALIGNMENT_CHECK,"Alignment Check");
EXCEPTION_MAKER(MACHINE_CHECK,"Machine Check");
EXCEPTION_MAKER(SIMD_FLOATING_POINT_EXCEPTION,"SIMD Floating-point exception");


//Description: Initializes IDT
//Inputs- None
//Outputs- None
// Return Value- None
//SIDE EFFECTS: initializes the IDT


	void exception_func(void){
			printf("This is exception\n");
	}

	//CREATE_FUNCTION(0);

void initialize_idt(void){
	int init_counter;

	for(init_counter=0;init_counter<NUM_EXCEPTION;init_counter++){//Initlizes the first 32 indices to exception values

		idt[init_counter].present=1; // sets present bit to 1 to show it is not empty
		idt[init_counter].dpl=0; //set dpl to 0 to indicate it has high priority
		idt[init_counter].reserved0=0; // this bits are to indicate the field 0S1110000000 where S is the size bit
		idt[init_counter].size=1; //Sets the gate size to 32 bits
		idt[init_counter].reserved1=1;
		idt[init_counter].reserved2=1;
		idt[init_counter].reserved3=1;
		idt[init_counter].reserved4 =0;
		idt[init_counter].seg_selector=KERNEL_CS; //Sets the segment selector to the Kernel's code segment
		SET_IDT_ENTRY(idt[init_counter],exception_func); //Sets the high and low 16 bit to test_interrupt
	}

	for(init_counter=NUM_EXCEPTION;init_counter<NUM_VEC; init_counter++){


			if(init_counter== SYS_CALL_INDEX){ //If the index is a systems call index continues onto the next interation fo the loop
					continue;
			}

			idt[init_counter].present=1; //Sets the prsent bit to 1 to show it is not empty
			idt[init_counter].dpl=0; //set dpl to 0 to indicate it has a high priority
			idt[init_counter].reserved0=0; //sets reserved this field is meant to indicate 0s110000000 where S is the size bit
			idt[init_counter].size=1; //sets the gate size ot 32 bits
			idt[init_counter].reserved1=1;
			idt[init_counter].reserved2=1;
			idt[init_counter].reserved3=0;
			idt[init_counter].reserved4=0;
			idt[init_counter].seg_selector=KERNEL_CS; //Sets the segment selector to Kernel's code segment
			SET_IDT_ENTRY(idt[init_counter],test_interrupt); //Sets the high and low 16 bits to test_interrupt
	}

	idt[SYS_CALL_INDEX].present=1; //Sets to present to indicate that is it present
	idt[SYS_CALL_INDEX].dpl=0; //Sets the gate size to 32 bits
	idt[SYS_CALL_INDEX].reserved0=0; //These bits are set to idicate the field 0s111000000 where S is the size bit
	idt[SYS_CALL_INDEX].size=1; //Sets the gate size to 32 bits
	idt[SYS_CALL_INDEX].reserved1=1;
	idt[SYS_CALL_INDEX].reserved2=1;
	idt[SYS_CALL_INDEX].reserved3=0;
	idt[SYS_CALL_INDEX].reserved4=0;
	idt[SYS_CALL_INDEX].seg_selector=KERNEL_CS; //Sets the segment selector to the Kernel's code segment
	SET_IDT_ENTRY(idt[SYS_CALL_INDEX],sys_call_handler); //Sets the high and low 16 bits to test_interrupt


  SET_IDT_ENTRY(idt[0],DIVIDE_ERROR);
	SET_IDT_ENTRY(idt[1],RESERVED);
  SET_IDT_ENTRY(idt[2],NMI);
  SET_IDT_ENTRY(idt[3],BREAKPOINT);
  SET_IDT_ENTRY(idt[4],OVERFLOW);
  SET_IDT_ENTRY(idt[5],BOUND_RANGE_EXCEEDED);
  SET_IDT_ENTRY(idt[6],INVALID_OPCODE);
  SET_IDT_ENTRY(idt[7],DEVICE_NOT);
  SET_IDT_ENTRY(idt[8], DOUBLE_FAULT);
  SET_IDT_ENTRY(idt[9], SEGMENT_OVERRUN);
  SET_IDT_ENTRY(idt[10], INVALID_TSS);
  SET_IDT_ENTRY(idt[11], SEGMENT_NOT_PRESENT);
  SET_IDT_ENTRY(idt[12], STACK_SEGMENT_FAULT);
  SET_IDT_ENTRY(idt[13], GENERAL_PROTECTION);
  SET_IDT_ENTRY(idt[14], PAGE_FAULT);
  SET_IDT_ENTRY(idt[16], MATH_FAULT);
  SET_IDT_ENTRY(idt[17], ALIGNMENT_CHECK);
  SET_IDT_ENTRY(idt[18], MACHINE_CHECK);
  SET_IDT_ENTRY(idt[19], SIMD_FLOATING_POINT_EXCEPTION);



    SET_IDT_ENTRY(idt[0x28], rtc_interrupt_handler);
  	SET_IDT_ENTRY(idt[0x20], irq0_handler);
	SET_IDT_ENTRY(idt[0x21], irq1_handler);
    SET_IDT_ENTRY(idt[0x22], irq2_handler);
    SET_IDT_ENTRY(idt[0x23], irq3_handler);
  	SET_IDT_ENTRY(idt[0x24], irq4_handler);
	SET_IDT_ENTRY(idt[0x25], irq5_handler);
	SET_IDT_ENTRY(idt[0x26], irq6_handler);
    SET_IDT_ENTRY(idt[0x27], irq7_handler);
    SET_IDT_ENTRY(idt[0x29], irq9_handler);
 	SET_IDT_ENTRY(idt[0x2A], irq10_handler);
    SET_IDT_ENTRY(idt[0x2B], irq11_handler);
    SET_IDT_ENTRY(idt[0x2C], irq12_handler);
  	SET_IDT_ENTRY(idt[0x2D], irq13_handler);
  	SET_IDT_ENTRY(idt[0x2E], irq14_handler);
  	SET_IDT_ENTRY(idt[0x2F], irq15_handler);
}
