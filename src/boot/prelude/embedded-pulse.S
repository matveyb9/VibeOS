/* VibeOS Prelude — embeds the independent Pulse payload in the UEFI loader. */

#ifndef PRELUDE_PULSE_BINARY
#error "PRELUDE_PULSE_BINARY must name the active Pulse raw image"
#endif

	.section .rdata,"dr"
	.global prelude_pulse_image_begin
	.global prelude_pulse_image_end
prelude_pulse_image_begin:
	.incbin PRELUDE_PULSE_BINARY
prelude_pulse_image_end:
