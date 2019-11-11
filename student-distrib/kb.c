#include "kb.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define IRQ_NUM           1
#define RECENT_RELEASE    0x80
#define LEFT_SHIFT        42
#define RIGHT_SHIFT       54
#define CAPS_LOCK         58
#define NEW_LINE          28
#define BACK_SPACE        14
#define CTRL              29

// some useful indices from the keyboard array
#define L_CHAR            38
#define Q_CHAR            16
#define P_CHAR            25
#define A_CHAR            30
#define L_CHAR            38
#define Z_CHAR            44
#define M_CHAR            50

#define CAP_OFFSET        90
#define UP_BOUND          128

#define BUF_LENGTH 128

unsigned long flags; /* Hold current flags */

// Flag whether or not the line buffer can be written to the screen or not
static volatile int32_t line_buffer_flag = 0;

volatile uint8_t kb_buf[BUF_LENGTH]; // Buffer of size that is 128;

static volatile int32_t buf_index = 0; //Index of the current element in the buffer

// Flags to indacte if shift, ctrl or caps lock is pressed
uint8_t shift_pressed = 0;
uint8_t caps_lock = 0;
uint8_t ctrl_pressed = 0;

// Table to map the scan_code not actualy 256 character in length
uint8_t kbdus[256] =
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
    0,	/* 69 - Num lock */
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
 * terminal_open
 *    DESCRIPTION: Function to be called to open the terminal driver
 *    INPUTS: None
 *    OUTPUTS: None
 *    Return Value: 0 for success
 *    SIDE EFFECTS: Initializes some of the global variables to 0
 */
int32_t terminal_open(const uint8_t* filename){
		buf_index = 0;
    return 0;
}

/*
 * terminal_write
 *    DESCRIPTION: Writes characters to the screen based on array that is passed in
 *    INPUTS: void* buf - Array to write to the screen
 *		        int32_t nbytes - The Number of bytes to write to the screen
 *    OUTPUTS: Character from copy_buf
 *    RETURN VALUE: -1 for failure, number of bytes written
 *    SIDE EFFECTS: None
 */
int terminal_write(int32_t fd, const void* buf, int32_t nbytes){
  /* Check for an invalid buffer */
	if(buf == NULL || nbytes < 0){
    /* Return failure */
		return -1;
	}

	int i; /* Loop variable */

  /* Disable interrupts */
  cli_and_save(flags);

  /* Print to the screen */
	for(i = 0; i < nbytes; i++){
		putc(*(uint8_t*)(buf+i));
	}

  /* Restore interrupts */
  restore_flags(flags);

  /* Number of bytes written */
	return nbytes;
}

/*
 * terminal_close
 *    DESCRIPTION: Closes the terminal driver
 *    INPUTS: int32_t fd - file to close
 *    OUTPUTS: None
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: None
 */
int32_t terminal_close(int32_t fd){
		buf_index = 0; // Sets the index back to 0
    return 0;
}

/*
 * terminal_read
 *    DESCRIPTION: Reads characters from the keyboard buffer into the buffer
 *    INPUTS: void* buf - Array to read into
 *		        int32_t nbytes - number of bytes to read into the buffer
 *    OUTPUTS: None
 *    RETURN VALUE: -1 for failure, Number of bytes read
 *    SIDE EFFECTS: None
 */
int terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int32_t bytes_read = 0; /* Number of bytes read */
		line_buffer_flag = 0; /* Sets the flag for determining whether enter is pressed */

		if(buf == NULL || nbytes < 0){
      /* Return failure */
			return -1;
		}

    /* Wait until the line is finsied */
		while(!line_buffer_flag){
		}

    /* Print the number of bytes desired or the number of bytes typed */
    bytes_read = nbytes < buf_index ? nbytes : buf_index;
    memcpy(buf, (void*)kb_buf, bytes_read);

    buf_index = 0;

    return bytes_read;
}

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


/* Some helper funstions for keyboard_interrupt_handler*/

/* caps_and_shift
 * checks is caps lock and shift are pressed
 * input: void
 * output: 1 or 0
 */
int32_t caps_and_shift (void) {
  return ((caps_lock == 1) && (shift_pressed == 1));
}

/* in_char_range
 * input: scan_code
 * output: 1 or 0
 * effects: checks if scan code is in between q to p (16-25), a to l (30-38), z to m (44-50)
 */
int32_t in_char_range (uint8_t scan_code) {
  return ((scan_code >= Q_CHAR && scan_code <= P_CHAR) || (scan_code >= A_CHAR && scan_code <= L_CHAR) || (scan_code >= Z_CHAR && scan_code <= M_CHAR));
}

/*
 * print_scancode
 *    DESCRIPTION: prints the scan_code
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: prints the matching characters of the scan_code
 */
void print_scancode (uint8_t scan_code) {
  if((buf_index >= (BUF_LENGTH - 1) && scan_code != NEW_LINE) || line_buffer_flag){
    return;
  }

  putc(kbdus[scan_code]); // Prints this scan code to the screen
  kb_buf[buf_index] = kbdus[scan_code];
  buf_index++;
}

/* caps_no_shift
 * checks is caps lock and shift not pressed
 * input: void
 * output: 1 or 0
 */
int caps_no_shift (void) {
  return ((caps_lock == 1) && (shift_pressed != 1));
}

/*
 * recent_release_exec
 *    DESCRIPTION: Executes when scan_code < RECENT_RELEASE. Handles the different scan_codes
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: none
 */
void recent_release_exec (uint8_t scan_code) {
  if(scan_code == CTRL){ //If the CTRL button is pressed
    ctrl_pressed = 1;
  }
  else if(scan_code == L_CHAR && ctrl_pressed == 1) {
    clear_l();
  }
  else if(scan_code == BACK_SPACE){
    /* Deletes a buffer character if it is allowed */
    if(buf_index <= 0){
      return;
    }
    buf_index--;
    back_space();
  }
  else if(scan_code == NEW_LINE){
    print_scancode(scan_code);
    line_buffer_flag = 1;
  }
  else if(scan_code == (LEFT_SHIFT) || scan_code == (RIGHT_SHIFT)){
    /* Sets shift_pressed to 1. Used ot indicate an on state */
    shift_pressed = 1;
  }
  else if(scan_code == CAPS_LOCK){
    // Mod 2 is so the values of capslock flip between 0 or 1
    caps_lock = (caps_lock + 1) % 2;
  }
  // Caps and shift are pressed and it is a letter
  else if(caps_and_shift() && in_char_range(scan_code)){
    print_scancode(scan_code);
  }
  // Caps on, shift off and it is a letter
  else if(caps_no_shift() && in_char_range(scan_code)){
    print_scancode(scan_code + CAP_OFFSET);
  }
  else if(shift_pressed == 1){
    print_scancode(scan_code + CAP_OFFSET);
  }
  else{
    print_scancode(scan_code);
  }
}

/*
 * after_release_exec
 *    DESCRIPTION: Executes when scan_code >= RECENT_RELEASE.
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Resets the flags for ctrl and shift to zero.
 */
void after_release_exec (uint8_t scan_code) {
  /* If the key is recently released, gets rid of the shift_pressed flag */
  if(scan_code == LEFT_SHIFT + RECENT_RELEASE || scan_code == RIGHT_SHIFT + RECENT_RELEASE){
    /* 0 is used to indicate a off state */
    shift_pressed = 0;
  }
  else if(scan_code == CTRL + RECENT_RELEASE){
    ctrl_pressed = 0;
  }
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
    /* Mask interrupts and store flags */
    cli_and_save(flags);
    /* Mask IRQ on PIC */
		disable_irq(IRQ_NUM);

    /* Send EOI to PIC to indicate interrupt is serviced */
		send_eoi(IRQ_NUM);

    /* Read the keyboard data buffer to get the current character */
		uint8_t scan_code = inb(0x60);

		if((scan_code >= CAP_OFFSET && scan_code <= UP_BOUND) || (scan_code >= (CAP_OFFSET + UP_BOUND))){
			return;
		}

    /* Tests to see whether the key is being presed versus being released */
		if(scan_code < RECENT_RELEASE){
      recent_release_exec(scan_code);
		} else{
      after_release_exec(scan_code);
		}

    /* Allow interrupts again */
		restore_flags(flags);
    /* Unmask the IRQ1 on the PIC */
		enable_irq(IRQ_NUM);
}
