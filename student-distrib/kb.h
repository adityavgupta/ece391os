/* kb.h - Defines interactions with keyboard interrupts */

#ifndef _KB_H
#define _KB_H

#ifndef ASM

/* Enable keyboard interrupts */
void keyboard_init(void);

/* Handler for keyboard interrupts */
void keyboard_interrupt_handler(void);

#endif /* ASM */

#endif /* _KB_H */
