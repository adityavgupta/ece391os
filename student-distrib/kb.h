/* kb.h - Defines interactions with keyboard interrupts */

#ifndef _KB_H
#define _KB_H

#include "types.h"

#ifndef ASM

/* Enable keyboard interrupts */
void keyboard_init(void);

//This is a redudnant fuctnion may need to remove
int should_stop(void);

//read the buffer inot the copy_buf array
int read(unsigned char* copy_buf,int nbytes);

//Open funciton te=o initnilize the driver
void open(void);
//Writes to the string buffer
int write(unsigned char* copy_buf,int nbytes);

//CLears the buffer
void clear_buf(void);
//Closes the terminal driver
void close(void);

// ctrl+L check
int ctrl_l (uint8_t scan_code);
// caps lock and shift
int caps_and_shift (void);

// caps lock and no shift
int caps_no_shift (void);

// in the ranges of alphabet in the keybaird array
int in_char_range (uint8_t scan_code);

// ctrl_l execution
void ctrl_l_exec (void);

// backspace execution
void backspace_exec (void);

// newline execution
void new_line_exec ();

// caps and sfit on execution
void caps_and_shift_exec (uint8_t scan_code);

// caps on, shift not on execution
void caps_no_shift_exec (uint8_t scan_code);

// shift execution
void shift_exec (uint8_t scan_code);

// printing the scan_code
void print_scancode (uint8_t scan_code);

// to execute when scan_code is less than RECENT_RELEASE
void recent_release_exec (uint8_t scan_code);

// to execute when scan_code is >= RECENT_RELEASE
void after_release_exec (uint8_t scan_code);

/* Handler for keyboard interrupts */
void keyboard_interrupt_handler(void);

#endif /* ASM */

#endif /* _KB_H */
