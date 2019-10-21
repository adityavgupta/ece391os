#define ASM 1
#include "kb.h"
#include "rtc.h"
#include "linkage.h"

.text
.globl keyboard_linkage, rtc_linkage


# Linkage for the keyboard handler
keyboard_linkage:
		# Save all registers
	  pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi

    call keyboard_interrupt_handler

		# Restore the registers
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
		# Return from interrupt
	  iret

# Linkage for the RTC handler
rtc_linkage:
		# Save all registers
	  pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi

    call rtc_interrupt_handler

		# Restore the registers
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
		# Return from interrupt
    iret
