#include "pcb.h"
#include "rtc.h"
#include "file_system.h"

/*
jump_table rtc_table={rtc_write,rtc_read,rtc_open,rtc_close};
jump_table file_table={file_write,file_read,file_open,file_close};
jump_table dir_table={dir_write,dir_read,dir_open,dir_close};*/

jump_table stdin_table={NULL,terminal_read,NULL,NULL};
jump_table stdout_table={terminal_write,NULL,NULL,NULL};
jump_table  descrip3={NULL,NULL,NULL,NULL};
jump_table  descrip4={NULL,NULL,NULL,NULL};
jump_table  descrip5={NULL,NULL,NULL,NULL};
jump_table  descrip6={NULL,NULL,NULL,NULL};
jump_table  descrip7={NULL,NULL,NULL,NULL};
jump_table  descrip8={NULL,NULL,NULL,NULL};
jump_table  descrip9={NULL,NULL,NULL,NULL};
jump_table  descrip10={NULL,NULL,NULL,NULL};
jump_table  descrip11={NULL,NULL,NULL,NULL};
jump_table  descrip12={NULL,NULL,NULL,NULL};



file_desc stdin_desc={&stdin_table,-1,-1,1};
file_desc stdout_desc={&stdout_table,-1,-1,1};
file_desc desc3={&descrip3,-1,-1,0};
file_desc desc4={&descrip4,-1,-1,0};
file_desc desc5={&descrip5,-1,-1,0};
file_desc desc6={&descrip6,-1,-1,0};
file_desc desc7={&descrip7,-1,-1,0};
file_desc desc8={&descrip8,-1,-1,0};
file_desc desc9={&descrip9,-1,-1,0};
file_desc desc10={&descrip10,-1,-1,0};
file_desc desc11={&descrip11,-1,-1,0};
file_desc desc12={&descrip12,-1,-1,0};


file_desc* first_file_desc_table[]={&stdin_desc,&stdout_desc,&desc3,&desc4,&desc5,&desc6,&desc7};
file_desc* second_file_desc_table[]={&stdin_desc,&stdout_desc,&desc8,&desc9,&desc10,&desc11,&desc12};

Pcb first_pcb={1,-1,0x7FFFFF,-1,-1,first_file_desc_table,STOPPED};
Pcb second_pcb={2,1,0x7FFFFF-(1024*8),0,0,second_file_desc_table,STOPPED};
