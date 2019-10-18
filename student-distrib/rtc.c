#include "lib.h"
#include "rtc.h"
#include "i8259.h"

void rtc_init(void){ //assume interrupts already disabled
  outb(REGISTER_B,RTC_PORT0); //select register B and disable NMI
  char prevB = inb(RTC_PORT1); //get previous register B value
  outb(REGISTER_B,RTC_PORT0); //select register B again
  outb(prevB|0x40,RTC_PORT0); //bitiwse or with 0x40 turns on bit 6 of register B to enable periodic interrupts by rtc
  enable_irq(2); //unmask slave irq on PIC
  enable_irq(RTC_IRQ_NUM); //enable rtc interrupt on PIC
}

void rtc_interrupt_handler(void){
  disable_irq(RTC_IRQ_NUM);//disable same interrupts
  send_eoi(RTC_IRQ_NUM);//send eoi to allow more interrupts
  test_interrupts(); //do some work
  sti(); //enable interrupts
  enable_irq(RTC_IRQ_NUM); //reenable same interrupts
}
