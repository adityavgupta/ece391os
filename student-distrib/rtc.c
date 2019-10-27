#include "lib.h"
#include "rtc.h"
#include "i8259.h"

/* Interrupt flag */
volatile uint32_t rtc_interrupt;

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

  //test_interrupts();
  uint8_t c = 'f';
  putc(c);

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
 * get_rate
 *    DESCRIPTION: Get rtc rate for the desired frequency
 *    INPUTS: int32_t freq - desired frequency
 *    OUTPUTS: none
 *    RETURN VALUE: rate to set rtc to get the desired frequency
 *    SIDE EFFECTS: None
 */
int32_t get_rate(int32_t freq){
  /* Rate of the RTC */
  int32_t rate = 0;

  /* Get log base 2 of the frequency */
  while(freq > 1){
      freq /= 2;
      rate++;
  }

  /* Rate begins at 15, so must subtract */
  return (15 - rate);
}

/*
 * rtc_open
 *    DESCRIPTION: Initialize RTC frequency
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Makes RTC rate 2Hz
 */
uint32_t rtc_open(const uint8_t* filename){
  unsigned long flags; /* Hold current flag values */

  /* Get the rate needed to set frequency to 2 Hz */
  int32_t rate = get_rate(2);

  /* Mask interrupts and save flags */
  cli_and_save(flags);

  outb(REGISTER_A, RTC_PORT0);

  /* Store old register A value */
  uint8_t prevA = inb(RTC_PORT1);
  outb(REGISTER_A, RTC_PORT0);

  /* Give the new rate to register A */
  outb((prevA & 0xF0)|rate , RTC_PORT1);

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
uint32_t rtc_close(int32_t fd){
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
uint32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
  /* Reset interrupt flag */
  rtc_interrupt = 0;
  printf("Wait for interrupt\n");
  /* Block until an RTC interrupt occurs */
  while(!rtc_interrupt){
    printf("Waiting");
  }
  printf("\nInterrupt occured");
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
uint32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
  unsigned long flags; /* Hold current flag values */

  int32_t rate = get_rate(nbytes);

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
