/* kb.h - Defines interactions with keyboard interrupts */

#ifndef _KB_H
#define _KB_H

#include "types.h"

#ifndef ASM

/* Enable keyboard interrupts */
void keyboard_init(void);

int should_stop(void);
 
int read(unsigned char* copy_buf,int nbytes);


void open(void);
int write(unsigned char* copy_buf,int nbytes);

void clear_buf(void);
void close(void);
 
/* Handler for keyboard interrupts */
void keyboard_interrupt_handler(void);

#endif /* ASM */

#endif /* _KB_H */
