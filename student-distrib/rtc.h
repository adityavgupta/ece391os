/* rtc.h - Defines and starts the rtc and handler */

#ifndef _RTC_H
#define _RTC_H

/* Ports and registers used to initialize the RTC. */
#define RTC_PORT0 0x70
#define RTC_PORT1 0x71
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define REGISTER_C 0x8C
#define SLAVE_PIN 2
#define RTC_IRQ_NUM 8

#ifndef ASM

/* Initialize RTC */
void rtc_init(void);

/* Handler for RTC interrupts */
void rtc_interrupt_handler(void);

/* Set the frequency of the RTC */
void set_rate(unsigned int rate);

#endif /* ASM */

#endif /* _RTC_H */
