/* kb.h - Defines interactions with keyboard interrupts */

#ifndef _KB_H
#define _KB_H

#include "types.h"

#ifndef ASM

/* Enable keyboard interrupts */
void keyboard_init(void);

// This is a redudnant fuctnion may need to remove
int should_stop(void);

// Read the buffer inot the copy_buf array
int read(uint8_t* copy_buf, int32_t nbytes);

// Writes to the string buffer
int write(uint8_t* copy_buf, int32_t nbytes);

// Open funciton te=o initnilize the driver
void open();

// Closes the terminal driver
void close();

// CLears the buffer
void clear_buf(void);

// ctrl+L check
int ctrl_l (uint8_t scan_code);
// caps lock and shift
int caps_and_shift (void);

// caps lock and no shift
int caps_no_shift (void);

// In the ranges of alphabet in the keybaird array
int in_char_range (uint8_t scan_code);

// ctrl_l execution
void ctrl_l_exec (void);

// Backspace execution
void backspace_exec (void);

// Newline execution
void new_line_exec ();

// Caps and sfit on execution
void caps_and_shift_exec (uint8_t scan_code);

// caps on, shift not on execution
void caps_no_shift_exec (uint8_t scan_code);

// Shift execution
void shift_exec (uint8_t scan_code);

// Printing the scan_code
void print_scancode (uint8_t scan_code);

// To execute when scan_code is less than RECENT_RELEASE
void recent_release_exec (uint8_t scan_code);

// To execute when scan_code is >= RECENT_RELEASE
void after_release_exec (uint8_t scan_code);

/* Handler for keyboard interrupts */
void keyboard_interrupt_handler(void);

#endif /* ASM */

#endif /* _KB_H */
