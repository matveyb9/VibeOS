/*
 * VibeOS Dawn ACPI metadata — validates one already-located RSDP only.
 * No ACPI root table, AML, hardware register, or firmware-service access is
 * performed here, so the routine is portable freestanding C17.
 */

#include "dawn_acpi.h"

static int dawn_acpi_checksum_is_zero(const uint8_t *bytes, uint32_t length) {
    uint8_t checksum = 0U;
    uint32_t index;

    for (index = 0U; index < length; ++index) {
        checksum = (uint8_t)(checksum + bytes[index]);
    }
    return checksum == 0U;
}

static uint32_t dawn_acpi_read_u32_le(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
           ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
}

int dawn_acpi_rsdp_is_valid(const void *physical_rsdp) {
    static const uint8_t signature[8] = {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};
    const uint8_t *rsdp = physical_rsdp;
    uint32_t index;
    uint32_t extended_length;

    if (rsdp == (const void *)0) {
        return 0;
    }
    for (index = 0U; index < sizeof(signature); ++index) {
        if (rsdp[index] != signature[index]) {
            return 0;
        }
    }
    if (!dawn_acpi_checksum_is_zero(rsdp, DAWN_ACPI_RSDP_V1_LENGTH)) {
        return 0;
    }
    if (rsdp[15] < 2U) {
        return 1;
    }

    extended_length = dawn_acpi_read_u32_le(&rsdp[20]);
    return extended_length == DAWN_ACPI_RSDP_V2_MINIMUM_LENGTH &&
           dawn_acpi_checksum_is_zero(rsdp, extended_length);
}
