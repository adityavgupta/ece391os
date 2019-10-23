#include "kb.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define IRQ_NUM 1
#define RECENT_RELEASE 0x80
#define LEFT_SHIFT 42
#define RIGHT_SHIFT 54
#define CAPS_LOCK 58

unsigned char shift_pressed = 0;
unsigned char caps_lock=0;
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



/*
 * keyboard_init
 *    DESCRIPTION: Enables keyboard interrupts
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Unmasks IRQ1 for keyboard interrupts on PIC
 */
void keyboard_init(void){
    /* Enable keyboard IRQ on PIC */
		enable_irq(IRQ_NUM);
}

/*
 * keyboard_interrupt_handler
 *    DESCRIPTION: Handler for keyboard interrupts
 *    INPUTS: none
 *    OUTPUTS: Character on the screen
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Prints keyboard input to screen, and sends EOI
 */
void keyboard_interrupt_handler(void){
    unsigned long flags; /* Hold current flags */

    /* Mask interrupts and store flags */
    cli_and_save(flags);
    /* Mask IRQ on PIC */
		disable_irq(IRQ_NUM);

    /* Send EOI to PIC to indicate interrupt is serviced */
		send_eoi(IRQ_NUM);

    /* Read the keyboard data buffer to get the current character */
		uint8_t scan_code = inb(0x60);
		
		//printf("%d\n",scan_code);

    /* Test if the character is to be printed or not */
		if(scan_code < RECENT_RELEASE){
        /* If the shift key is pressed adds 128 to index into capital characters 128 is the for the additional 128 character when they are capitalized */
				//putc(kbdus[scan_code]);
        /* Checks to see if the shift key has been recently released or not */
				//printf(" %d ",scan_code);
				if(scan_code == (LEFT_SHIFT) || scan_code == (RIGHT_SHIFT)){
          /* Sets to shift pressed to be pressed 1 is used ot indicate an on state */
					shift_pressed = 1;
					//printf(" Shift has been pressed ");
				} else if(scan_code==CAPS_LOCK){
					caps_lock=(caps_lock+1)%2;
					//printf(" %d ",caps_lock);
				}else if((caps_lock==1) != (shift_pressed==1)){
					putc(kbdus[scan_code+90]); //90 is the offset required to obtain the capital letters
				} else{
					putc(kbdus[scan_code]);
				}
		} else{
      /* If the key is recntly released gets rid of the shift_pressed flag */
			if(scan_code == LEFT_SHIFT+RECENT_RELEASE || scan_code == RIGHT_SHIFT+RECENT_RELEASE ){
        /* 0 is used ot indicate a off state */
				shift_pressed = 0;
				//printf(" Shift has been released ");
			}
		}

    /* Allow interrupts again */
		restore_flags(flags);
    /* Unmask the IRQ1 on the PIC */
		enable_irq(IRQ_NUM);
}
