#define ASM 1
#include "kb.h"
#include "rtc.h"
#include "linkage.h"

.text
.globl keyboard_linkage, rtc_linkage, system_call_handler

# System call linkage
system_call_handler:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
		call *system_call_table(, %eax, 4)
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
