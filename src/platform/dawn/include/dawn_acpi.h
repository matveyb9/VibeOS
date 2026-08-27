/*
 * VibeOS Dawn ACPI metadata — a bounded RSDP integrity check at the
 * Prelude-to-Pulse boundary. This is deliberately not an ACPI table parser.
 */

#ifndef VIBEOS_DAWN_ACPI_H
#define VIBEOS_DAWN_ACPI_H

#include <stdint.h>

#define DAWN_ACPI_RSDP_V1_LENGTH UINT32_C(20)
#define DAWN_ACPI_RSDP_V2_MINIMUM_LENGTH UINT32_C(36)

int dawn_acpi_rsdp_is_valid(const void *physical_rsdp);

#endif
