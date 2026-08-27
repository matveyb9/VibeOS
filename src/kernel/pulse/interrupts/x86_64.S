/* VibeOS Pulse — minimal no-return early exception handlers for QEMU proof. */
	.section .text.interrupts,"ax"
	.extern pulse_panic_unhandled_interrupt
	.global pulse_x86_default_interrupt
	.type pulse_x86_default_interrupt,@function
pulse_x86_default_interrupt:
	cli
	andq $-16, %rsp
	subq $8, %rsp
	movl $0xffffffff, %edi
	call pulse_panic_unhandled_interrupt
1:
	hlt
	jmp 1b

	.global pulse_x86_breakpoint_interrupt
	.type pulse_x86_breakpoint_interrupt,@function
pulse_x86_breakpoint_interrupt:
	cli
	leaq pulse_x86_breakpoint_message(%rip), %rsi

pulse_x86_debug_exit:
	cld
	movw $0x402, %dx
1:
	lodsb
	testb %al, %al
	jz 2f
	outb %al, %dx
	jmp 1b
2:
	xorl %eax, %eax
	movw $0x0f4, %dx
	outl %eax, %dx
3:
	hlt
	jmp 3b

	.section .rodata,"a"
pulse_x86_breakpoint_message:
	.asciz "PULSE: breakpoint trap handled\n"
