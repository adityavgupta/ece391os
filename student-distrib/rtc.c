#include "lib.h"
#include "rtc.h"
#include "i8259.h"


asm volatile rtc_interrupt;

/*
 * rtc_init
 *    DESCRIPTION: Initializes RTC
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Changes register B and register C, and enables RTC interrupts on PIC
 */
void rtc_init(void){
    /* Disable non-maskable interrupts and select register B */
    outb(REGISTER_B, RTC_PORT0);

    /* Store the current register B value */
    uint8_t prevB = inb(RTC_PORT1);

    /* Select register B again */
    outb(REGISTER_B, RTC_PORT0);

    /* 0x40 allows periodic RTC interrupts */
    outb(prevB | 0x40, RTC_PORT1);

    /* Enable RTC PIC interrupts */
    enable_irq(RTC_IRQ_NUM);
}

/*
 * rtc_interrupt_handler
 *    DESCRIPTION: Handler for rtc interrupts
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Throws away RTC input and sends EOI
 */
void rtc_interrupt_handler(void){
  unsigned long flags; /* Hold the current flags */

  /* Mask interrupt flags */
  cli_and_save(flags);
  /* Turn off PIC interrupts */
  disable_irq(RTC_IRQ_NUM);

  /* Send EOI signal */
  send_eoi(RTC_IRQ_NUM);

  rtc_interrupt = 1;

  test_interrupts();

  /* Read register C */
  outb(REGISTER_C, RTC_PORT0);

  /* Clear contents of RTC to allow RTC interrupts again */
  inb(RTC_PORT1);

  /* Re-enable interrupts and restores flags */
  restore_flags(flags);
  /* Unmask PIC interrupts*/
  enable_irq(RTC_IRQ_NUM);
}

/*
 * rtc_open
 *    DESCRIPTION: Initialize RTC frequency
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Makes RTC rate 2Hz
 */
uint32_t rtc_open(void){
  unsigned long flags; /* Hold current flag values */

  /* Mask interrupts and save flags */
  cli_and_save(flags);

  outb(REGISTER_A, RTC_PORT0);

  /* Store old register A value */
  uint8_t prevA = inb(RTC_PORT1);
  outb(REGISTER_A, RTC_PORT0);

  /* Give the new rate to register A */
  outb((prevA & 0xF0)| 15, RTC_PORT1);

  /* Restore the interrupt flags */
  restore_flags(flags);

  /* Return success */
  return 0;
}

/*
 * rtc_close
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS:
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
uint32_t rtc_close(){
  /* Do nothing for now, until virtualization is added */
  return 0;
}

/*
 * rtc_read
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS:
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
uint32_t rtc_read(){
  unsigned long flags; /* Hold curent flag values */

  /* Block until an RTC interrupt occurs */
  while(!rtc_interrupt){
    /* Block interrupts */
    cli_and_save(flags);

    /* Reset interrupt counter */
    rtc_interrupt = 0;

    /* Restore interrupts */
    restore_flags(flags);
  }

  /* Return success */
  return 0;
}

/*
 * rtc_write
 *    DESCRIPTION: Change the RTC rate
 *    INPUTS: uint32_t rate - the frequency to set
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, or -1 for failure
 *    SIDE EFFECTS: Changes the frequency the RTC operates at
 */
uint32_t rtc_write(uint32_t rate){
  unsigned long flags; /* Hold current flag values */

  /* Rate must be valid */
  if(rate > 15 || rate < 3){
    /* Return failure */
    return -1;
  }

  /* Get 4 most significant bits */
  rate &= 0x0F;

  /* Mask interrupts and save flags */
  cli_and_save(flags);
  outb(REGISTER_A, RTC_PORT0);

  /* Store old register A value */
  uint8_t prevA = inb(RTC_PORT1);
  outb(REGISTER_A, RTC_PORT0);

  /* Give the new rate to register A */
  outb((prevA & 0xF0)| rate, RTC_PORT1);

  /* Restore the interrupt flags */
  restore_flags(flags);

  /* Return success */
  return 0;
}
