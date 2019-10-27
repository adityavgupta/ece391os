/* rtc.h - Defines and starts the rtc and handler */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* Ports and registers used to initialize the RTC. */
#define RTC_PORT0 0x70
#define RTC_PORT1 0x71
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define REGISTER_C 0x8C
#define SLAVE_PIN 2
#define RTC_IRQ_NUM 8

#ifndef ASM

/* Flag to allow prints for test cases */
extern uint32_t rtc_test_flag;

/* Initialize RTC */
void rtc_init(void);

/* Handler for RTC interrupts */
void rtc_interrupt_handler(void);

/* RTC device driver open */
uint32_t rtc_open(const uint8_t* filename);

/* RTC device driver close */
uint32_t rtc_close(int32_t fd);

/* RTC device driver read */
uint32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Set the frequency of the RTC */
uint32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Get rtc rate needed for desired frequency*/
int32_t get_rate(int32_t freq);

#endif /* ASM */

#endif /* _RTC_H */
