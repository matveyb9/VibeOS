/* Dawn ACPI RSDP test — checks bounded signature, version, and checksum rules. */

#include <stdint.h>
#include <stdio.h>

#include "dawn_acpi.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

static void apply_checksum(uint8_t *bytes, uint32_t length, uint32_t checksum_offset) {
    uint8_t checksum = 0U;
    uint32_t index;

    bytes[checksum_offset] = 0U;
    for (index = 0U; index < length; ++index) {
        checksum = (uint8_t)(checksum + bytes[index]);
    }
    bytes[checksum_offset] = (uint8_t)(0U - checksum);
}

static void make_valid_rsdp_v2(uint8_t rsdp[36]) {
    uint32_t index;
    static const uint8_t signature[8] = {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};

    for (index = 0U; index < 36U; ++index) {
        rsdp[index] = 0U;
    }
    for (index = 0U; index < sizeof(signature); ++index) {
        rsdp[index] = signature[index];
    }
    rsdp[15] = 2U;
    rsdp[20] = 36U;
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
}

static void write_u32_le(uint8_t *bytes, uint32_t value) {
    uint32_t index;

    for (index = 0U; index < 4U; ++index) {
        bytes[index] = (uint8_t)(value >> (index * 8U));
    }
}

static void write_u64_le(uint8_t *bytes, uint64_t value) {
    uint32_t index;

    for (index = 0U; index < 8U; ++index) {
        bytes[index] = (uint8_t)(value >> (index * 8U));
    }
}

static void make_root_table(uint8_t *table, uint32_t length, const uint8_t signature[4]) {
    uint32_t index;

    for (index = 0U; index < length; ++index) {
        table[index] = 0U;
    }
    for (index = 0U; index < 4U; ++index) {
        table[index] = signature[index];
    }
    write_u32_le(&table[4], length);
    table[8] = 1U;
    apply_checksum(table, length, 9U);
}

int main(void) {
    static uint8_t rsdp[36];
    static uint8_t xsdt[44];
    static uint8_t rsdt[44];
    DAWN_ACPI_ROOT_TABLE_METADATA metadata;
    static const uint8_t xsdt_signature[4] = {'X', 'S', 'D', 'T'};
    static const uint8_t rsdt_signature[4] = {'R', 'S', 'D', 'T'};

    make_valid_rsdp_v2(rsdp);
    if (!expect(!dawn_acpi_rsdp_is_valid((const void *)0), "null pointer is rejected") ||
        !expect(dawn_acpi_rsdp_is_valid(rsdp), "valid ACPI 2.0 RSDP is accepted")) {
        return 1;
    }
    rsdp[0] = 'X';
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "wrong RSDP signature is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[20] = 35U;
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "short ACPI 2.0 RSDP length is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[8] = (uint8_t)(rsdp[8] + 1U);
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "bad legacy checksum is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[32] = (uint8_t)(rsdp[32] + 1U);
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "bad extended checksum is rejected")) {
        return 1;
    }

    make_root_table(xsdt, sizeof(xsdt), xsdt_signature);
    make_valid_rsdp_v2(rsdp);
    write_u64_le(&rsdp[24], (uint64_t)(uintptr_t)xsdt);
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
    if (!expect(dawn_acpi_root_table_describe(rsdp, &metadata), "valid XSDT is described") ||
        !expect(metadata.kind == DAWN_ACPI_ROOT_TABLE_XSDT && metadata.physical_address == (uint64_t)(uintptr_t)xsdt &&
                    metadata.byte_size == sizeof(xsdt) && metadata.entry_count == 1U,
                "XSDT metadata preserves root address, length, and entry count")) {
        return 1;
    }
    make_root_table(rsdt, sizeof(rsdt), rsdt_signature);
    write_u64_le(&rsdp[24], 0U);
    write_u32_le(&rsdp[16], (uint32_t)(uintptr_t)rsdt);
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
    if (!expect(dawn_acpi_root_table_describe(rsdp, &metadata), "RSDT fallback is described") ||
        !expect(metadata.kind == DAWN_ACPI_ROOT_TABLE_RSDT && metadata.byte_size == sizeof(rsdt) &&
                    metadata.entry_count == 2U,
                "RSDT metadata retains 32-bit entry width")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    write_u64_le(&rsdp[24], (uint64_t)(uintptr_t)xsdt);
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
    xsdt[0] = 'Y';
    if (!expect(!dawn_acpi_root_table_describe(rsdp, &metadata), "wrong root signature is rejected")) {
        return 1;
    }

    puts("Dawn ACPI RSDP unit tests passed.");
    return 0;
}
