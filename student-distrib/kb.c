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

#define ZERO 0
#define ONE 1
#define BUFF_LENGTH 128

//extern unsigned char buf[128];
int flag=ZERO; // this is longer needed
unsigned long flags; /* Hold current flags */

static volatile int line_buffer_flag=ZERO; // Flag whether or not the line buffer can be written to the screen or not
//extern int flag_1;

volatile unsigned char buf[BUFF_LENGTH]={ZERO}; // Buffer of size that is 128;
volatile unsigned char prev_buf[BUFF_LENGTH]; //Stores the elements prevous stored in the buffer for the read function

static int volatile buf_index=ZERO; //Index of the current element in the buffer
static int volatile prev_index=ZERO; //Previous index to indicates where in length
int flag_1=ZERO; //These two variables are redundant will remove later

int str_len_count;

unsigned char shift_pressed = ZERO; //Flags to indacte if shift, ctrl or caps lock is pressed
unsigned char caps_lock=ZERO;
unsigned char ctrl_pressed=ZERO;
unsigned char kbdus[256] = // Table to map the scan_code not actualy 256 character in length
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
 * */
void clear_buf(void){
		int buf_counter;  //counter to loop through the array

		for(buf_counter=0; buf_counter<=BUFF_LENGTH;buf_counter++){ //Loops from 0 to one to clear the buffer while also cpying the array
				prev_buf[buf_counter]=buf[buf_counter];
				buf[buf_counter]=ZERO;
		}
		prev_index=buf_index; //Sotres the index of the previous buffer
		str_len_count=ZERO; //This is redudant will need to remove later
		buf_index=ZERO; //Sets the index back to 0
}

/* open
 * Description- Function to be called to open teh terminal driver
 * Inputs- None
 * Outputs-None
 * Return Value-None
 * Side Efects- Initilizes osme of the gloabal variables to 0
 * */

void open(void){
		str_len_count=ZERO; // intilalizes the buf _index to 0, str_len_count is redudant and may need to be removed
		buf_index=ZERO;
}

/* keyboard_helper
 * Description- Helper function to print variables onto the screen
 * Inputs- scan code- variable to indicates the ascii to be printed
 * Output- None
 * Return Value- None
 * Side effects- Prints value onto the screen
 * */
void keyboard_helper(uint8_t scan_code){

				if(buf_index>=BUFF_LENGTH){ /*If the buf_index is greater than the length of the buffer it will clear the buffer*/
					clear_buf();
				}
				if(scan_code=='\n'){ //If the new line character is present will move on to the next line while clearing the buffer
					//printf("\n");
					new_line();
					buf[buf_index]=scan_code;
					buf_index++;
					clear_buf();
				}
				 else{
           putc(scan_code); //Print sthe character to the screen
           buf[buf_index]=scan_code; //Puts the value into the buffer
           buf_index++;
				}
}

/* write
 * Description- writes characters to the screen based on array that is passed in
 * Inputs- copy_buf- Array to write to the screen
 *		  nbytes- The Number of bytes to write to the screen
 * Outputs- None
 * Return Value- Returns 1 for sucess or the number of bytes written
 * Side Effects- Character from copy_buf will be written the screen
 * */
int write(unsigned char* copy_buf,int nbytes){

	if(copy_buf==NULL){ // If a null pointer is passed in return -1 to indicate failure
			return -1;
	}
	int strlength=nbytes/sizeof(unsigned char); // caluclatues the string length based on the number of bytes and the size of the unisigned char

	int i;
	for(i=0;i<strlength;i++){ //Using the keyboard_helper function to print to the screen
		keyboard_helper(copy_buf[i]);
	}

	return nbytes; //returns the number of bytes

}

/* close
 * Description- closes the terminal driver
 * Inputs- None
 * Outputs- None
 * Return Value- None
 * Side Effects- None
 * */
void close(void){
		int i;
		for(i=0;i<BUFF_LENGTH;i++){ //intilizes the buffer back to 0
			buf[i]=ZERO; // Buffer of size that is 128;
		}
		buf_index=ZERO; //Initilzies the index into the back to 0
}

/* read
 * Description-Reads character from the buffer into the array
 * Inputs- copy_buf- Array to read into
 *		  nbytes- number of bytes to read into teh array
 * Outputs- None
 * Return Value- Number ofbytes able to be read
 * Side-Effects
 * */
int read(unsigned char* copy_buf,int nbytes){

		line_buffer_flag=ZERO; //Sets the flag for determining whether the flag is finished
		if(copy_buf==NULL){ //If a null pointer was placed retruns 0 bytes
			return ZERO;
		}
		while(line_buffer_flag==ZERO){ //spins until the line has finsied being typed by the keyboard handler

		}

		//printf("%c",prev_buf[0]);

		//printf("%d %d %d",nbytes,prev_index,buf_index);

		if(nbytes< prev_index*sizeof(unsigned char)){ //Determines whether to print the number of bytes desired or the number of bytes typed
				memcpy(copy_buf,(void*)prev_buf,nbytes);
				return nbytes;
		} else{
				memcpy(copy_buf,(void*)prev_buf,prev_index*sizeof(unsigned char));
				return prev_index*sizeof(unsigned char);
		}

}




/* keyboard_init
 *    DESCRIPTION: Enables keyboard interrupts
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Unmasks IRQ1 for keyboard interrupts on PIC
 */
void keyboard_init(void){
    /* Enable keyboard IRQ on PIC */
		enable_irq(IRQ_NUM); //eneables the itreuppts on the keyboard
		//buf[0]='\n';
}


/* Some helper funstions for keyboard_interrupt_handler*/

/* ctrl_l
 * input: None
 * output: 1 or 0
 * effect: checks is ctrl+l has been pressed
 */
int ctrl_l (uint8_t scan_code) {
  return (scan_code==L_CHAR && ctrl_pressed==ONE);
}

/* caps_and_shift
 * checks is caps lock and shift are pressed
 * input: void
 * output: 1 or 0
 */
int caps_and_shift (void) {
  return ((caps_lock==ONE)&&(shift_pressed==ONE));
}

/* caps_no_shift
 * checks is caps lock and shift not pressed
 * input: void
 * output: 1 or 0
 */
int caps_no_shift (void) {
  return ((caps_lock==ONE) && (shift_pressed!=ONE));
}

/* in_char_range
 * input: scan_code
 * output: 1 or 0
 * effects: checks if scan code is in between q to p (16-25), a to l (30-38), z to m (44-50)
 */
int in_char_range (uint8_t scan_code) {
  return ((scan_code>= Q_CHAR && scan_code<=P_CHAR)||(scan_code>=A_CHAR && scan_code<=L_CHAR)||(scan_code>=Z_CHAR &&scan_code<=M_CHAR));
}

/*
 * ctrl_l_exec
 *    DESCRIPTION: Executes when ctrl+l is pressed
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: cleares the screen
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
  int temp=back_space(); //tests to see if the back_space has reached the beginning of the line or not
  if(temp==ZERO){ //deletes the buffer variable during the back space if it is allowed
    buf[buf_index]=ZERO;
    buf_index--;

    if(buf_index<ZERO){
        buf_index=ZERO;
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
  buf[buf_index]=kbdus[NEW_LINE]; //Places the new line in the buffer
  buf_index++;
  line_buffer_flag=ONE;  //Sets the flag to one to let the read stop spinning
  clear_buf(); //Clears the buffer
}

/*
 * caps_and_shift_exec
 *    DESCRIPTION: Executes when caps and shift are pressed together
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: prints the matching characters of the scan_code when caps and shift are pressed
 */
void caps_and_shift_exec (uint8_t scan_code) {
  putc(kbdus[scan_code]);
  buf[buf_index]=kbdus[scan_code];
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
  putc(kbdus[scan_code+CAP_OFFSET]); //90 is the offset required to obtain the capital letters
  buf[buf_index]=kbdus[scan_code+CAP_OFFSET];
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
  putc(kbdus[scan_code+CAP_OFFSET]); //90 is the offset required to obtain the capital letters
  buf[buf_index]=kbdus[scan_code+CAP_OFFSET];
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
  putc(kbdus[scan_code]); //Prints this scan code to the screen
  buf[buf_index]=kbdus[scan_code];
  buf_index++;
}

/*
 * recent_release_exec
 *    DESCRIPTION: Executes when scan_code < RECENT_RELEASE. Handles the different scan_codes
 *    INPUTS: uint32_t scan_code - scan_code of the key pressed
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS:
 */
void recent_release_exec (uint8_t scan_code) {
  if(scan_code==CTRL){ //If the CTRL button is pressed
    ctrl_pressed=ONE;
  }
  // ctrl+l
  else if(ctrl_l(scan_code)) {
    ctrl_l_exec ();
  }
  else if(scan_code== BACK_SPACE){
    backspace_exec ();
  }
  else if(scan_code==NEW_LINE){
    new_line_exec ();
  }
  else if(scan_code == (LEFT_SHIFT) || scan_code == (RIGHT_SHIFT)){
    /* Sets shift_pressed to 1. Used ot indicate an on state */
    shift_pressed = ONE;
  }
  else if(scan_code==CAPS_LOCK){
    // Mod 2 is so the values of capslock flip between 0 or 1
    caps_lock=(caps_lock+ONE)%2;
  }
  // caps and shift are pressed and it is a letter
  else if(caps_and_shift() && in_char_range(scan_code)){
      caps_and_shift_exec (scan_code);
  }
  // caps on, shift off and it is a letter
  else if(caps_no_shift() && in_char_range(scan_code)){
      caps_no_shift_exec (scan_code);
  }

  else if((shift_pressed==ONE)){
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
  /* If the key is recntly released gets rid of the shift_pressed flag */
  if(scan_code == LEFT_SHIFT+RECENT_RELEASE || scan_code == RIGHT_SHIFT+RECENT_RELEASE ){

    /* 0 is used ot indicate a off state */
    shift_pressed = ZERO;
  }
  else if(scan_code==CTRL+RECENT_RELEASE){
      ctrl_pressed=ZERO;
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
    cli_and_save(flags); //Masks the falgs and disable the interrupts

    /* Mask IRQ on PIC */
		disable_irq(IRQ_NUM);

    /* Send EOI to PIC to indicate interrupt is serviced */
		send_eoi(IRQ_NUM);

    /* Read the keyboard data buffer to get the current character */
		uint8_t scan_code = inb(0x60);

		if((scan_code>=CAP_OFFSET && scan_code<=UP_BOUND)||(scan_code>=(CAP_OFFSET+UP_BOUND))){
			return;
		}

    /* Test if the character is to be printed or not */
		if(scan_code < RECENT_RELEASE){//Tests to see whether the key is being presed versus being released
      recent_release_exec (scan_code);
		}
    else{
      after_release_exec (scan_code);
		}

		if(buf_index>=BUFF_LENGTH){
					line_buffer_flag=ONE; //Sets the flag for the event the buf_index is great than the buffer length
					clear_buf(); // Clears the buffer
		}
    /* Allow interrupts again */
		restore_flags(flags);
    /* Unmask the IRQ1 on the PIC */
		enable_irq(IRQ_NUM);
}
