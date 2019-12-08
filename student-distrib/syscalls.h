#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "kb.h"
#include "file_system.h"
#include "rtc.h"
#include "linkage.h"
#include "lib.h"
#include "paging.h"

/* Maximum number of file descriptor indexes */
#define MAX_FD_NUM 7
#define EIGHT_KB        0x2000
#define EIGHT_MB        0x800000
#define FOUR_MB         0x400000
#define USER_PROG       0x8000000

#ifndef ASM

/* Number of active processes */
extern int32_t process_num;

/* Address of first instruction of shell */
uint32_t program_addr_test;

/* Jump table for system calls */
typedef struct jump_table{
	int32_t(*write)(int32_t, const void*, int32_t);
	int32_t(*read)(int32_t, void*, int32_t);
	int32_t(*open)(const uint8_t*);
	int32_t(*close)(int32_t);
} jump_table;

/* File descriptor struct */
typedef struct file_desc{
	jump_table* jump_ptr;     /* Jump table to file's system calls */
	int32_t inode;            /* File inode number */
	int32_t file_position;    /* Current position of the file */
	int32_t flags;            /* Flag to indicate is a descriptor is in use */
} file_desc;

/* PCB struct,  */
typedef struct{
  int32_t pid; 									/* Process identification number */
  int32_t parent_pid; 					/* Parent process identification number */
  int32_t current_esp; 					/* Current esp */
  int32_t parent_esp; 					/* Parent's esp */
  int32_t parent_ebp; 					/* Parent's ebp */
  file_desc fdt[MAX_FD_NUM+1]; 	/* File Descriptor Table */
  uint8_t process_state;  /* State of process */
	uint8_t args[BUF_LENGTH];     /* Commands passed in */
	int32_t vidmem;
} pcb_t;

int32_t launch(void);

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

/* Copy program commands to userspace */
int32_t getargs(uint8_t* buf, int32_t nbytes);

/* Map video memory to userspace */
int32_t vidmap(uint8_t** screen_start);

/* */
int32_t set_handler(int32_t signum, void* handler_address);

/* */
int32_t sigreturn(void);

/* Function for bad read system calls */
int32_t invalid_read(int32_t fd, void* buf, int32_t nbytes);

/* Function for bad write system calls */
int32_t invalid_write(int32_t fd, const void* buf, int32_t nbytes);

/* Gets the address of the current pcb */
pcb_t* get_pcb_add(void);

#endif /* ASM */

#endif /* _SYSCALLS_H */
