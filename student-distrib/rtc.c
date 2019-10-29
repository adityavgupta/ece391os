#include "lib.h"
#include "rtc.h"
#include "i8259.h"

/* Interrupt flag */
volatile uint32_t rtc_interrupt;

/* Flag to allow prints for test cases */
uint32_t rtc_test_flag = 0;

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
  if(rtc_test_flag){
    uint8_t c = 'f';
    putc(c);
  }

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
 *    RETURN VALUE: -1 for failure, rate to set rtc to get the desired frequency
 *    SIDE EFFECTS: None
 */
int32_t get_rate(int32_t freq){
  /* Rate of the RTC */
  int32_t rate = 0;

  /* Get log base 2 of the frequency */
  while(freq > 1){
    /* Check if power of 2 */
    if(freq % 2 != 0){
      /* Return failure */
      return -1;
    }
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

  /* Mask interrupts and save flags */
  cli_and_save(flags);

  /* Get the rate needed to set frequency to 2 Hz */
  int32_t rate = get_rate(2);

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
 *    DESCRIPTION: Closes the file
 *    INPUTS: int32_t fd - file to close
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: none
 */
uint32_t rtc_close(int32_t fd){
  /* Do nothing for now, until virtualization is added */
  return 0;
}

/*
 * rtc_read
 *    DESCRIPTION: Waits for an RTC interrupt to occur
 *    INPUTS: int32_t fd - the file to read
 *            void* buf - a buffer, does nothing
 *            int32_t - number of bytes in the buffer
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: none
 */
uint32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
  /* Reset interrupt flag */
  rtc_interrupt = 0;

  /* Block until an RTC interrupt occurs */
  while(!rtc_interrupt){

  }

  /* Return success */
  return 0;
}
/*
 * rtc_write
 *    DESCRIPTION: Change the RTC rate
 *    INPUTS: int32_t fd - file to write to
 *            const void* buf - frequency to set
 *            int32_t nbytes - size of the buffer
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, or -1 for failure
 *    SIDE EFFECTS: Changes the frequency the RTC operates at
 */
uint32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
  int32_t freq; /* Frequency from the buffer */
  int32_t rate; /* Rate from frequency */

  /* Check for valid inputs */
  if(nbytes != 4 || (int32_t*)buf == NULL){
    /* Return failure */
    return -1;
  }

  /* Get the frequency */
  freq = *(int32_t*)buf;

  /* Check if frequency is valid */
  if(freq < 0 || freq > 1024){
    /* Return failure */
    return -1;
  }

  /* Check if frequency is a power of 2 */
  if(-1 == (rate = get_rate(freq))){
    /* Return failure */
    return -1;
  }

  /* Get 4 least significant bits */
  rate &= 0x0F;

  outb(REGISTER_A, RTC_PORT0);

  /* Store old register A value */
  uint8_t prevA = inb(RTC_PORT1);
  outb(REGISTER_A, RTC_PORT0);

  /* Give the new rate to register A */
  outb((prevA & 0xF0)| rate, RTC_PORT1);

  /* Return success */
  return 0;
}
