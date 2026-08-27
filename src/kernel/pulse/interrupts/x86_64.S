/* VibeOS Pulse — minimal no-return early exception handlers for QEMU proof. */
	.section .text.interrupts,"ax"
	.extern pulse_panic_unhandled_interrupt
	.extern atlas_i8042_keyboard_handle_irq1
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
	jmp pulse_x86_debug_exit

	.global pulse_x86_timer_interrupt
	.type pulse_x86_timer_interrupt,@function
pulse_x86_timer_interrupt:
	cli
	leaq pulse_x86_timer_message(%rip), %rsi
	jmp pulse_x86_debug_exit

	.global pulse_x86_keyboard_interrupt
	.type pulse_x86_keyboard_interrupt,@function
pulse_x86_keyboard_interrupt:
	cli
	cld
	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %rsi
	pushq %rdi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call atlas_i8042_keyboard_handle_irq1
	testl %eax, %eax
	jnz pulse_x86_keyboard_event_handled
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rdi
	popq %rsi
	popq %rdx
	popq %rcx
	popq %rax
	iretq

pulse_x86_keyboard_event_handled:
	leaq pulse_x86_keyboard_message(%rip), %rsi

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
pulse_x86_timer_message:
	.asciz "PULSE: timer interrupt handled\n"
pulse_x86_keyboard_message:
	.asciz "ATLAS: keyboard IRQ1 event handled\n"
