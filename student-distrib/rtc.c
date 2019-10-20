#include "lib.h"
#include "rtc.h"
#include "i8259.h"
#include "types.h"
void rtc_init(void){ //assume interrupts already disabled
  outb(REGISTER_B,RTC_PORT0); //select register B and disable NMI
  char prevB = inb(RTC_PORT1); //get previous register B value
  outb(REGISTER_B,RTC_PORT0); //select register B again
  outb(prevB|0x40,RTC_PORT1); //bitiwse or with 0x40 turns on bit 6 of register B to enable periodic interrupts by rtc
  enable_irq(SLAVE_PIN); //unmask slave irq on PIC
  enable_irq(RTC_IRQ_NUM); //enable rtc interrupt on PIC
}

void rtc_interrupt_handler(void){
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(RTC_IRQ_NUM);//disable same interrupts
  send_eoi(RTC_IRQ_NUM);//send eoi to allow more interrupts
  //printf("rtc interrupt occurred\n");
  test_interrupts(); //do some work
  outb(REGISTER_C,RTC_PORT0); //read register C
  inb(RTC_PORT1);//throw contents away, so that rtc interrupts can happen again
  restore_flags(flags); //enable interrupts
  enable_irq(RTC_IRQ_NUM); //reenable same interrupts
}

void set_rate(unsigned int rate){
  if(rate>15||rate<3)return; //invalid rates, do nothing
  unsigned long flags;
  rate&=0x0F;//only keep first 4 bits, since rtc rate is bottom 4 bits of register A
  cli_and_save(flags); //cli and save flags
  outb(REGISTER_A,RTC_PORT0);
  char prevA = inb(RTC_PORT1);//get old register A value
  outb(REGISTER_A,RTC_PORT0);
  outb((prevA&0xF0)|rate,RTC_PORT1); //write new rate to register A
  restore_flags(flags);//restore flags
}
