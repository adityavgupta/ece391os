#define RTC_PORT0 0x70
#define RTC_PORT1 0x71
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define REGISTER_C 0x8C
#define RTC_IRQ_NUM 8
#define SLAVE_PIN 2

#ifndef ASM
void rtc_init(void);
void rtc_interrupt_handler(void);
#endif
