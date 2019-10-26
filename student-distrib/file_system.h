#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"


// directory entry struct
typedef struct dentry_struct dentry_t;

// boot block
typedef struct boot_struct boot_block_t;

void file_system_init(uint32_t* file_sys_start);
dentry_t* find_dentry(const uint8_t* filename);
int32_t read_dentry_by_name(const uint8_t* filename, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t file_open();
int32_t file_close();
int32_t file_read(int32_t fd, void* buf, uint32_t num_bytes);
int32_t file_write();
int32_t dir_open(const uint8_t* filename);
int32_t dir_close();
int32_t dir_read();
int32_t dir_write();

#endif
