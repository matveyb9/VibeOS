/* VibeOS Prelude — embeds the independent Pulse payload in the UEFI loader. */
	.section .rdata,"dr"
	.global prelude_pulse_image_begin
	.global prelude_pulse_image_end
prelude_pulse_image_begin:
	.incbin "build/pulse/pulse.bin"
prelude_pulse_image_end:
