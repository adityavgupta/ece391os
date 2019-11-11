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
    # Point ds to user stack segment
    movw $USER_DS, %ax
    movw %ax, %ds
    pushf
    popl %eax
    orl $0x200, %eax # Set IF to 1 for user space
    pushl %eax
    popf
    pushl %ds # Push user ds
    pushl $0x83FFFFF # Push stack pointer of user program
    pushf # Push EFLAGS
    pushl $USER_CS  # Push user cs
    pushl %ecx # User eip
    iret  # Switch to user space

# System call linkage
system_call_handler:
    sti # enable interrupts in kernel space
    # Save registers
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    subl $1, %eax # System call numbers are from 1-6, map them to 0-5 for jump table
		cmpl $5, %eax # Check is sys call number greater than 5, if so its invalid
		ja SYSCALL_DONE
		cmpl $0, %eax # Check is sys call number less than 0, if so its invalid
		jb SYSCALL_DONE
		call *system_call_table(, %eax, 4)  # Call the appropriate system call
SYSCALL_DONE:
    # Restore registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
		iret  # switch back to user space

# Jump table for system call
system_call_table:
	.long halt, execute, read, write, open, close

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
