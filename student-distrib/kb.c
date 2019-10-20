#include "kb.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define IRQ_NUM 1
#define RECENT_RELEASE 0x80
#define LEFT_SHIFT 42
#define RIGHT_SHIFT 52
unsigned char shift_pressed=0;
unsigned char kbdus[256] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
	    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};


//Will need to go back to get rid of magic numbers


//Description- Initilizes the keyboard for use
//Inputs- None
//Outputs-None
//Return Value- None
//Side Effects- None

void keyboard_init(void){

		enable_irq(1); //enables te IRQ for the keyboard which is mapped to IRQ 1

}

//Description- Interuupt_handler for th event that the keyboard is pressed
//Inputs- None
//Outpus- none
//Return Value- none
//Side Effects- prints a character to the screen depending on the key tht has beeen pressed

void keyboard_interrupt_handler(void){
    unsigned long flags;
    cli_and_save(flags);
		disable_irq(IRQ_NUM); //Disables the IRQ
		send_eoi(IRQ_NUM); //Sends the EOI signal
		unsigned char scan_code= inb(0x60); //Writes to the port 0x60 the port used to as the data buffer for the keyboard

		if(!(scan_code & RECENT_RELEASE)){ // tests if the character is to be printed or not

				putc(kbdus[scan_code]+128*shift_pressed);	//If the shift key is pressed adds 128 to index into capital characters 128 is the for the additonal 128 character when they are captialized

				if(scan_code==(RECENT_RELEASE+LEFT_SHIFT) || scan_code==( RECENT_RELEASE+RIGHT_SHIFT)){ //Checks to see if the shift key has been recently released or not

						shift_pressed=1; //Sets to shift pressed to be pressed 1 is used ot indicate an on state

				}
		} else{
				if(scan_code== LEFT_SHIFT || scan_code== RIGHT_SHIFT ){ // If the key is recntly released gets rid of the shift_pressed flag
						shift_pressed=0; // 0 is used ot indicate a off state
				}

		}
		restore_flags(flags); // Allows keyboard interrupts to occur again
		enable_irq(IRQ_NUM); //Enables the interrupts to IRQ #1



}
