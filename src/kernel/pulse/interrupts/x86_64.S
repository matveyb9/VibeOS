/* VibeOS Pulse — minimal no-return early exception handlers for QEMU proof. */
	.section .text.interrupts,"ax"
	.global pulse_x86_default_interrupt
	.type pulse_x86_default_interrupt,@function
pulse_x86_default_interrupt:
	cli
	leaq pulse_x86_default_message(%rip), %rsi
	jmp pulse_x86_debug_exit

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
pulse_x86_default_message:
	.asciz "PULSE: unexpected interrupt\n"
pulse_x86_breakpoint_message:
	.asciz "PULSE: breakpoint trap handled\n"
