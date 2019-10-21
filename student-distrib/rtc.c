#include "lib.h"
#include "rtc.h"
#include "i8259.h"
#include "types.h"

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

void set_rate(unsigned int rate){
  unsigned long flags; /* Hold current flag values */

  /* Rate must be valid */
  if(rate > 15 || rate < 3){
    return ;
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
}
