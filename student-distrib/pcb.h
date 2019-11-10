#ifndef _PCB_H
#define _PCB_H

#include "syscall.h"
#define RUNNING 0
#define STOPPED 1

#ifndef ASM

typedef struct{
  int32_t pid; //process identification number
  int32_t parent_pid; //parent process identitifcation nubmer
  int32_t current_esp; //current_esp
  int32_t parent_esp; //Parent's esp
  int32_t parent_ebp; //parent's ebp
  file_desc** fdt; //File Descriptor Table
  unsigned char process_state;
} Pcb;

#endif /* ASM */

#endif /* _PCB_H */
