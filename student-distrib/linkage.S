#define ASM 1
#include "kb.h"
#include "rtc.h"
#include "linkage.h"
#include "syscalls.h"
#include "x86_desc.h"

.text
.globl keyboard_linkage, rtc_linkage, system_call_handler, context_switch

# Switch to user space
context_switch:
    mov ds $USER_DS
    mov cs $USER_CS
    pushl %ds
    pushl $0x083FFFFF
    pushf
    pushl %cs
    pushl 16(%esp) # user eip

    iret

# System call linkage
system_call_handler:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

		cmpl $5, %eax
		ja SYSCALL_DONE
		cmpl $0, %eax
		jb SYSCALL_DONE
		call *system_call_table(, %eax, 4)
SYSCALL_DONE:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
		iret

system_call_table:
	.long  halt, execute, read, write, open, close

# Linkage for the keyboard handler
keyboard_linkage:
		# Save all registers
		pushl %eax
	  pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    call keyboard_interrupt_handler

		# Restore the registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
		# Return from interrupt
	  iret

# Linkage for the RTC handler
rtc_linkage:
		# Save all registers
		pushl %eax
	  pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    call rtc_interrupt_handler

		# Restore the registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
		# Return from interrupt
    iret
