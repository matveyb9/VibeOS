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

static uint64_t dawn_acpi_read_u64_le(const uint8_t *bytes) {
    uint64_t value = 0U;
    uint32_t index;

    for (index = 0U; index < 8U; ++index) {
        value |= (uint64_t)bytes[index] << (index * 8U);
    }
    return value;
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

int dawn_acpi_root_table_describe(const void *physical_rsdp, DAWN_ACPI_ROOT_TABLE_METADATA *metadata) {
    static const uint8_t rsdt_signature[4] = {'R', 'S', 'D', 'T'};
    static const uint8_t xsdt_signature[4] = {'X', 'S', 'D', 'T'};
    const uint8_t *rsdp = physical_rsdp;
    const uint8_t *root_table;
    const uint8_t *expected_signature;
    uint64_t root_table_address;
    uint32_t root_table_length;
    uint32_t entry_width;
    uint32_t index;
    uint8_t root_kind;

    if (metadata == (void *)0) {
        return 0;
    }
    metadata->physical_address = 0U;
    metadata->byte_size = 0U;
    metadata->entry_count = 0U;
    metadata->kind = 0U;
    if (!dawn_acpi_rsdp_is_valid(rsdp)) {
        return 0;
    }
    root_table_address = dawn_acpi_read_u32_le(&rsdp[16]);
    root_kind = DAWN_ACPI_ROOT_TABLE_RSDT;
    entry_width = 4U;
    expected_signature = rsdt_signature;
    if (rsdp[15] >= 2U && dawn_acpi_read_u64_le(&rsdp[24]) != 0U) {
        root_table_address = dawn_acpi_read_u64_le(&rsdp[24]);
        root_kind = DAWN_ACPI_ROOT_TABLE_XSDT;
        entry_width = 8U;
        expected_signature = xsdt_signature;
    }
    if (root_table_address == 0U) {
        return 0;
    }
    root_table = (const uint8_t *)(uintptr_t)root_table_address;
    for (index = 0U; index < sizeof(rsdt_signature); ++index) {
        if (root_table[index] != expected_signature[index]) {
            return 0;
        }
    }
    root_table_length = dawn_acpi_read_u32_le(&root_table[4]);
    if (root_table_length < DAWN_ACPI_TABLE_HEADER_LENGTH ||
        root_table_length > DAWN_ACPI_ROOT_TABLE_MAXIMUM_LENGTH ||
        ((root_table_length - DAWN_ACPI_TABLE_HEADER_LENGTH) % entry_width) != 0U ||
        !dawn_acpi_checksum_is_zero(root_table, root_table_length)) {
        return 0;
    }
    metadata->physical_address = root_table_address;
    metadata->byte_size = root_table_length;
    metadata->entry_count = (root_table_length - DAWN_ACPI_TABLE_HEADER_LENGTH) / entry_width;
    metadata->kind = root_kind;
    return 1;
}
