#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "kb.h"
#include "file_system.h"
#include "rtc.h"
#include "linkage.h"
#include "lib.h"

#ifndef ASM

/* Number of active processes */
extern int32_t process_num;

/* Jump table for system calls */
typedef struct jump_table{
	int32_t(*write)(int32_t,const void*,int32_t);
	int32_t(*read)(int32_t,void*,int32_t);
	int32_t(*open)(const uint8_t*);
	int32_t(*close)(int32_t);

} jump_table;

/* File descriptor struct */
typedef struct file_desc{
	jump_table* jump_ptr;
	int32_t inode;
	int32_t file_position;
	int32_t flags;

} file_desc;

/* PCB struct,  */
typedef struct{
  int32_t pid; 									/* Process identification number */
  int32_t parent_pid; 					/* Parent process identification number */
  int32_t current_esp; 					/* Current esp */
  int32_t parent_esp; 					/* Parent's esp */
  int32_t parent_ebp; 					/* Parent's ebp */
  file_desc fdt[8]; 						/* File Descriptor Table */
  unsigned char process_state;  /* State of process */
} pcb_t;

/* Halt system call, stop a process */
int32_t halt(uint8_t status);

/* Execute system call, begins a process */
int32_t execute(const uint8_t* command);

/* Read system call, reads from a file */
int32_t read(int32_t fd, void* buf, int32_t nbytes);

/* Write system call, writes to a file */
int32_t write(int32_t fd, const void* buf, int32_t nbytes);

/* Open system call, opens a file descriptor */
int32_t open(const uint8_t* filename);

/* Close system call, closes a file descriptor */
int32_t close(int32_t fd);

/* Gets the address of the current pcb */
pcb_t* get_pcb_add (void);

#endif /* ASM */

#endif /* _SYSCALLS_H */
