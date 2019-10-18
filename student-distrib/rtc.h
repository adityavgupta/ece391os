#define RTC_PORT0 0x70
#define RTC_PORT1 0x71
#define REGISTER_B 0x8B
#define RTC_IRQ_NUM 8

void rtc_init(void);
void rtc_interrupt_handler(void);
