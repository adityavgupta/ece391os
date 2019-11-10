#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "kb.h"
#include "file_system.h"
#include "rtc.h"
#include "linkage.h"
#include "lib.h"

#ifndef ASM

typedef struct jump_table{
	int32_t(*write)(int32_t,const void*,int32_t);
	int32_t(*read)(int32_t,void*,int32_t);
	int32_t(*open)(const uint8_t*);
	int32_t(*close)(int32_t);
	
}jump_table;

typedef struct file_desc{
	jump_table* jump_ptr;
	int32_t inode;
	int32_t file_position;
	int32_t flags;
	
} file_desc;

int32_t halt(uint8_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, const void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

#endif /* ASM */

#endif /* _SYSCALLS_H */
