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

static void make_madt(uint8_t *table) {
    static const uint8_t signature[4] = {'A', 'P', 'I', 'C'};

    make_root_table(table, 64U, signature);
    write_u32_le(&table[36], UINT32_C(0xfee00000));
    write_u32_le(&table[40], 1U);
    table[44] = 0U;
    table[45] = 8U;
    table[52] = 1U;
    table[53] = 12U;
    apply_checksum(table, 64U, 9U);
}

static int identity_reader(uint64_t physical_address, uint32_t byte_count, uint8_t *destination, void *context) {
    const uint8_t *source = (const uint8_t *)(uintptr_t)physical_address;
    uint32_t index;

    (void)context;
    if (source == (const void *)0 || destination == (void *)0) {
        return 0;
    }
    for (index = 0U; index < byte_count; ++index) {
        destination[index] = source[index];
    }
    return 1;
}

int main(void) {
    static uint8_t rsdp[36];
    static uint8_t xsdt[44];
    static uint8_t xsdt_children[52];
    static uint8_t xsdt_capacity[556];
    static uint8_t rsdt[44];
    static uint8_t child_madt[64];
    static uint8_t child_mcfg[36];
    DAWN_ACPI_ROOT_TABLE_METADATA metadata;
    DAWN_ACPI_CHILD_TABLE_INVENTORY inventory;
    DAWN_ACPI_MADT_INVENTORY madt_inventory;
    static const uint8_t xsdt_signature[4] = {'X', 'S', 'D', 'T'};
    static const uint8_t rsdt_signature[4] = {'R', 'S', 'D', 'T'};
    static const uint8_t mcfg_signature[4] = {'M', 'C', 'F', 'G'};

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
    make_madt(child_madt);
    make_root_table(child_mcfg, sizeof(child_mcfg), mcfg_signature);
    make_root_table(xsdt_children, sizeof(xsdt_children), xsdt_signature);
    write_u64_le(&xsdt_children[36], (uint64_t)(uintptr_t)child_madt);
    write_u64_le(&xsdt_children[44], (uint64_t)(uintptr_t)child_mcfg);
    apply_checksum(xsdt_children, sizeof(xsdt_children), 9U);
    make_valid_rsdp_v2(rsdp);
    write_u64_le(&rsdp[24], (uint64_t)(uintptr_t)xsdt_children);
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
    if (!expect(dawn_acpi_root_table_describe(rsdp, &metadata), "XSDT with children is described") ||
        !expect(dawn_acpi_child_table_inventory(&metadata, identity_reader, (void *)0, &inventory),
                "XSDT child headers are inventoried") ||
        !expect(inventory.entry_count == 2U && inventory.omitted_entry_count == 0U &&
                    inventory.entries[0].physical_address == (uint64_t)(uintptr_t)child_madt &&
                    inventory.entries[0].signature[0] == 'A' && inventory.entries[0].signature[3] == 'C' &&
                    inventory.entries[0].byte_size == sizeof(child_madt) &&
                    inventory.entries[0].revision == 1U &&
                    inventory.entries[0].status == (DAWN_ACPI_CHILD_TABLE_HEADER_VALID | DAWN_ACPI_CHILD_TABLE_CHECKSUM_VALID) &&
                    inventory.entries[1].signature[0] == 'M' && inventory.entries[1].signature[3] == 'G',
                "child records retain address signature length revision and checksum state")) {
        return 1;
    }
    if (!expect(dawn_acpi_madt_inventory((uint64_t)(uintptr_t)child_madt, identity_reader, (void *)0, &madt_inventory),
                    "valid MADT is inventoried") ||
        !expect(madt_inventory.byte_size == sizeof(child_madt) &&
                    madt_inventory.local_interrupt_controller_address == UINT32_C(0xfee00000) &&
                    madt_inventory.flags == 1U && madt_inventory.entry_count == 2U &&
                    madt_inventory.entries[0].type == 0U && madt_inventory.entries[0].length == 8U &&
                    madt_inventory.entries[1].type == 1U && madt_inventory.entries[1].length == 12U,
                    "MADT retains only fixed metadata and bounded entry headers")) {
        return 1;
    }
    child_madt[53] = 0U;
    apply_checksum(child_madt, sizeof(child_madt), 9U);
    if (!expect(!dawn_acpi_madt_inventory((uint64_t)(uintptr_t)child_madt, identity_reader, (void *)0, &madt_inventory),
                    "zero-length MADT entry is rejected")) {
        return 1;
    }
    make_madt(child_madt);
    child_mcfg[9] = (uint8_t)(child_mcfg[9] + 1U);
    if (!expect(dawn_acpi_child_table_inventory(&metadata, identity_reader, (void *)0, &inventory) &&
                    inventory.entries[1].status == DAWN_ACPI_CHILD_TABLE_HEADER_VALID,
                "invalid child checksum is recorded without payload parsing")) {
        return 1;
    }
    make_root_table(xsdt_capacity, sizeof(xsdt_capacity), xsdt_signature);
    make_valid_rsdp_v2(rsdp);
    write_u64_le(&rsdp[24], (uint64_t)(uintptr_t)xsdt_capacity);
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
    if (!expect(dawn_acpi_root_table_describe(rsdp, &metadata) &&
                    dawn_acpi_child_table_inventory(&metadata, identity_reader, (void *)0, &inventory) &&
                    inventory.entry_count == DAWN_ACPI_CHILD_TABLE_CAPACITY && inventory.omitted_entry_count == 1U,
                "child inventory reports bounded capacity omission")) {
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
