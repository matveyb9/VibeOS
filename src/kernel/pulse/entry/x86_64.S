/* VibeOS Pulse — explicit x86_64 entry symbol for the first raw kernel image. */
	.section .text.entry,"ax"
	.global _pulse_entry
	.type _pulse_entry,@function
_pulse_entry:
	cli
	cld
	movq %rdi, %r12
	movq 56(%r12), %rsp
	andq $-16, %rsp
	subq $8, %rsp
	movq %r12, %rdi
	jmp pulse_entry
