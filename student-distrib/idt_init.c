#include "idt_init.h"
#include "x86_desc.h"
#include "rtc.h"
#include "lib.h"

# define NUM_EXCEPTION 32
# define SYS_CALL_INDEX 0x80


//Description- Random test function
//INPUTS-None
//Outputs-None
//Return Value- none
//Side Effects- prints the string "Steven Lumetta is an AI coded by Steven Lumetta" to the screen

void test_interrupt(void){
	printf("Steve Lumetta is an AI coded by Steven Lumetta\n");
}

//Description: Initializes IDT
//Inputs- None
//Outputs- None
// Return Value- None
//SIDE EFFECTS: initializes the IDT


#define CREATE_FUNCTION(x)\
	void exception_func(void){\
			printf("This is exception %d",x)\
	}

//CREATE_FUNCTION(0);

void initialize_idt(void){
	int init_counter;

	for(init_counter=0;init_counter<NUM_EXCEPTION;init_counter++){

		idt[init_counter].present=1; // sets present bit to 1 to show it is not empty
		idt[init_counter].dpl=0; //set dpl to 0 to indicate it has high priority
		idt[init_counter].reserved0=0; // this bits are to indicate the field 0S1110000000 where S is the size bit
		idt[init_counter].size=1; //Sets the gate size to 32 bits
		idt[init_counter].reserved1=1;
		idt[init_counter].reserved2=1;
		idt[init_counter].reserved3=1;
		idt[init_counter].reserved4 =0;
		idt[init_counter].seg_selector=KERNEL_CS; //Sets the segment selector to the Kernel's code segment
		SET_IDT_ENTRY(idt[init_counter],test_interrupt); //Sets the high and low 16 bit to test_interrupt
	}

	for(init_counter=NUM_EXCEPTION;init_counter<NUM_VEC; init_counter++){ //Initlizes the first 32 indices to exception values


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
	SET_IDT_ENTRY(idt[SYS_CALL_INDEX],test_interrupt); //Sets the high and low 16 bits to test_interrupt

	SET_IDT_ENTRY(idt[0x28],rtc_interrupt_handler); //set rtc handler




}
