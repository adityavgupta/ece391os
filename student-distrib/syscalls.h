#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "kb.h"
#include "file_system.h"
#include "rtc.h"

#ifndef ASM

int32_t halt(uint8_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

#endif /* ASM */

#endif /* _SYSCALLS_H */
