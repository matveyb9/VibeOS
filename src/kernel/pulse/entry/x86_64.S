/* VibeOS Pulse — explicit x86_64 entry symbol for the first raw kernel image. */
	.section .text.entry,"ax"
	.global _pulse_entry
	.type _pulse_entry,@function
_pulse_entry:
	cli
	jmp pulse_entry
