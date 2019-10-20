#define ASM 1
#include "kb.h"
#include "rtc.h"
#include "linkage.h"

.text
.globl keyboard_linkage, rtc_linkage


keyboard_linkage:

	pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi

    call keyboard_interrupt_handler

    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
	iret

rtc_linkage:
	pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi

    call rtc_interrupt_handler

    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret
