/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

static mask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

/* Initialize the 8259 PIC */
void i8259_init(void) {
    master_mask = 0xFF;
    slave_mask = 0xFF;

    outb(master_mask, MASTER_8259_PORT+1);
    outb(slave_mask, SLAVE_8259_PORT+1);

    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT+1);
    outb(ICW3_MASTER, MASTER_8259_PORT+1);
    outb(ICW4, MASTER_8259_PORT+1);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1);
    outb(ICW4, SLAVE_8259_PORT+1);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
  if(irq_num < 8){
    master_mask |=  mask[irq_num];
    outb(master_mask, MASTER_8259_PORT+1);
  } else {
    slave_mask |= mask[irq_num];
    outb(slave_mask, SLAVE_8259_PORT+1);
  }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  if(irq_num < 8){
    master_mask &= !mask[irq_num];
    outb(master_mask, MASTER_8259_PORT+1);
  } else {
    slave_mask &= !mask[irq_num];
    outb(slave_mask, SLAVE_8259_PORT+1);
  }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
  if(irq_num < 8){
    outb(EOI, MASTER_8259_PORT);
  } else {
    outb(EOI, SLAVE_8259_PORT);
  }
}
