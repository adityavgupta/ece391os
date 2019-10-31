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

volatile uint8_t kb_buf[BUF_LENGTH] = {'\0'}; // Buffer of size that is 128;

static volatile int32_t buf_index = 0; //Index of the current element in the buffer

//Flags to indacte if shift, ctrl or caps lock is pressed
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

/* clear_buf
 * Description: This function will clear the buffer used for line buffering
 * INputs-None
 * Outputs- None
 * Return VAlue- None
 * Side Effects- Clears the buf array
 */
void clear_buf(void){
		int32_t buf_counter;  // counter to loop through the array

    // Set buffer to 0
		for(buf_counter = 0; buf_counter < BUF_LENGTH; buf_counter++){
			buf[buf_counter] = '\0';
		}
		buf_index = 0; // Sets the index back to 0
}

/*
 * terminal_open
 *    DESCRIPTION: Function to be called to open the terminal driver
 *    INPUTS: None
 *    OUTPUTS: None
 *    Return Value: None
 *    SIDE EFFECTS: Initializes some of the global variables to 0
 */
void open(){
		buf_index = 0;
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
int terminal_write(uint8_t* buf, int32_t nbytes){
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
		putc(buf[i]);
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
 *    RETURN VALUE: None
 *    SIDE EFFECTS: None
 */
void terminal_close(){
		int i;

		for(i = 0; i < BUF_LENGTH; i++){ //intilizes the buffer back to 0
			kb_buf[i] = '\0'; // Buffer of size that is 128;
		}

		buf_index = 0; //Initilzies the index into the back to 0
}

/*
 * terminal_read
 *    DESCRIPTION: Reads characters from the keyboard buffer into the buffer
 *    INPUTS: void* buf - Array to read into
 *		        int32_t nbytes - number of bytes to read into the buffer
 *    OUTPUTS: None
 *    RETURN VALUE: Number of bytes read
 *    SIDE EFFECTS: None
 */
int terminal_read(uint8_t* buf, int32_t nbytes){
    int32_t bytes_read = 0; /* Number of bytes read */
		line_buffer_flag = 0; //Sets the flag for determining whether enter is pressed

		if(buf == NULL){ //If a null pointer was placed returns 0 bytes
			return -1;
		}

    // Spins until the line is finsied
		while(!line_buffer_flag){
		}

    // Determines whether to print the number of bytes desired or the number of bytes typed
		if(nbytes < buf_index){
			memcpy(buf, (void*)kb_buf, nbytes);
			bytes_read = nbytes;
		} else{
			memcpy(buf, (void*)kb_buf, buf_index);
			bytes_read = buf_index;
		}

    clear_buf(); //Clears the buffer

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

/* ctrl_l
 * input: None
 * output: 1 or 0
 * effect: checks is ctrl+l has been pressed
 */
int ctrl_l (uint8_t scan_code) {
  return (scan_code == L_CHAR && ctrl_pressed == ONE);
}

/* caps_and_shift
 * checks is caps lock and shift are pressed
 * input: void
 * output: 1 or 0
 */
int caps_and_shift (void) {
  return ((caps_lock == ONE) && (shift_pressed == ONE));
}

/* caps_no_shift
 * checks is caps lock and shift not pressed
 * input: void
 * output: 1 or 0
 */
int caps_no_shift (void) {
  return ((caps_lock == ONE) && (shift_pressed != ONE));
}

/* in_char_range
 * input: scan_code
 * output: 1 or 0
 * effects: checks if scan code is in between q to p (16-25), a to l (30-38), z to m (44-50)
 */
int in_char_range (uint8_t scan_code) {
  return ((scan_code >= Q_CHAR && scan_code <= P_CHAR) || (scan_code >= A_CHAR && scan_code <= L_CHAR) || (scan_code >= Z_CHAR && scan_code <= M_CHAR));
}

/*
 * ctrl_l_exec
 *    DESCRIPTION: Executes when ctrl+l is pressed
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Clears the screen
 */
void ctrl_l_exec (void) {
  reset_screen(); //resets the screen and clears the buffer and the screen
  clear_buf();
  clear();
}

/*
 * backspace_exec
 *    DESCRIPTION: Executes when backspace is pressed
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: deletes the character previously typed
 */
void backspace_exec (void) {
  int temp = back_space(); //tests to see if the back_space has reached the beginning of the line or not
  if(temp == 0){ //deletes the buffer variable during the back space if it is allowed
    kb_buf[buf_index] = 0;
    buf_index--;

    if(buf_index < 0){
        buf_index = 0;
    }
  }
}

/*
 * new_line_exec
 *    DESCRIPTION: Executes when enter is pressed
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: change to new line on hitting enter
 */
void new_line_exec () {
  new_line(); //Goes to new line scrolls if necessary
  buf[buf_index] = kbdus[NEW_LINE]; //Places the new line in the buffer
  buf_index++;
  line_buffer_flag = 1;  //Sets the flag to one to let the read stop spinning
}

/*
 * caps_and_shift_exec
 *    DESCRIPTION: Executes when caps and shift are pressed together
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: Prints the matching characters of the scan_code when caps and shift are pressed
 *    RETURN VALUE: none
 *    SIDE EFFECTS: none
 */
void caps_and_shift_exec (uint8_t scan_code) {
  putc(kbdus[scan_code]);
  buf[buf_index] = kbdus[scan_code];
  buf_index++;
}

/*
 * caps_no_shift_exec
 *    DESCRIPTION: Executes when caps is pressed and no shift
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: prints the matching characters of the scan_code when caps is pressed and no shift
 */
void caps_no_shift_exec (uint8_t scan_code) {
  putc(kbdus[scan_code + CAP_OFFSET]); //90 is the offset required to obtain the capital letters
  buf[buf_index] = kbdus[scan_code + CAP_OFFSET];
  buf_index++;
}

/*
 * shift_exec
 *    DESCRIPTION: Executes when shift is pressed
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: prints the matching characters of the scan_code when shift is pressed
 */
void shift_exec (uint8_t scan_code) {
  putc(kbdus[scan_code + CAP_OFFSET]); //90 is the offset required to obtain the capital letters
  buf[buf_index] = kbdus[scan_code + CAP_OFFSET];
  buf_index++;
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
  if(buf_index >= (BUF_LENGTH - 1))

  putc(kbdus[scan_code]); //Prints this scan code to the screen
  buf[buf_index] = kbdus[scan_code];
  buf_index++;
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
  else if(ctrl_l(scan_code)) {
    ctrl_l_exec ();
  }
  else if(scan_code == BACK_SPACE){
    backspace_exec ();
  }
  else if(scan_code == NEW_LINE){
    new_line_exec ();
  }
  else if(scan_code == (LEFT_SHIFT) || scan_code == (RIGHT_SHIFT)){
    /* Sets shift_pressed to 1. Used ot indicate an on state */
    shift_pressed = 1;
  }
  else if(scan_code == CAPS_LOCK){
    // Mod 2 is so the values of capslock flip between 0 or 1
    caps_lock = (caps_lock + 1) % 2;
  }
  // caps and shift are pressed and it is a letter
  else if(caps_and_shift() && in_char_range(scan_code)){
    caps_and_shift_exec (scan_code);
  }
  // caps on, shift off and it is a letter
  else if(caps_no_shift() && in_char_range(scan_code)){
    caps_no_shift_exec (scan_code);
  }

  else if((shift_pressed == 1)){
    shift_exec (scan_code);
  }
  else{
    print_scancode (scan_code);
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
  if(scan_code == LEFT_SHIFT + RECENT_RELEASE || scan_code == RIGHT_SHIFT + RECENT_RELEASE ){
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
      recent_release_exec (scan_code);
		} else{
      after_release_exec (scan_code);
		}

		if(buf_index >= BUF_LENGTH){
      /* Sets the flag for the event the buf_index is great than the buffer length */
			line_buffer_flag = 1;
		}
    /* Allow interrupts again */
		restore_flags(flags);
    /* Unmask the IRQ1 on the PIC */
		enable_irq(IRQ_NUM);
}
