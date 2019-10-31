/* kb.h - Defines interactions with keyboard interrupts */

#ifndef _KB_H
#define _KB_H

#include "types.h"

#ifndef ASM

/* Enable keyboard interrupts */
void keyboard_init(void);

// Read the buffer inot the copy_buf array
int32_t terminal_read(uint8_t* buf, int32_t nbytes);

// Writes to the string buffer
int32_t terminal_write(uint8_t* buf, int32_t nbytes);

// Open funciton te=o initnilize the driver
void terminal_open();

// Closes the terminal driver
void terminal_close();

// CLears the buffer
void clear_buf(void);

// ctrl+L check
int32_t ctrl_l (uint8_t scan_code);

// caps lock and shift
int32_t caps_and_shift (void);

// In the ranges of alphabet in the keybaird array
int32_t in_char_range (uint8_t scan_code);

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
