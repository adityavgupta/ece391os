#include "pcb.h"
#include "rtc.h"
#include "file_system.h"

/*
jump_table rtc_table={rtc_write,rtc_read,rtc_open,rtc_close};
jump_table file_table={file_write,file_read,file_open,file_close};
jump_table dir_table={dir_write,dir_read,dir_open,dir_close};*/

jump_table stdint_table = {NULL, terminal_read, NULL, NULL};
jump_table stdoutt_table = {terminal_write, NULL, NULL, NULL};

file_desc stdin_desc = {&stdint_table, -1, -1, 1};
file_desc stdout_desc = {&stdoutt_table, -1, -1, 1};
