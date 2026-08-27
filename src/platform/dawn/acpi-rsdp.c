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

static void dawn_acpi_child_inventory_initialize(DAWN_ACPI_CHILD_TABLE_INVENTORY *inventory) {
    uint32_t entry_index;
    uint32_t byte_index;

    inventory->entry_count = 0U;
    inventory->omitted_entry_count = 0U;
    for (entry_index = 0U; entry_index < DAWN_ACPI_CHILD_TABLE_CAPACITY; ++entry_index) {
        inventory->entries[entry_index].physical_address = 0U;
        inventory->entries[entry_index].byte_size = 0U;
        inventory->entries[entry_index].revision = 0U;
        inventory->entries[entry_index].status = 0U;
        for (byte_index = 0U; byte_index < 4U; ++byte_index) {
            inventory->entries[entry_index].signature[byte_index] = 0U;
        }
    }
}

static void dawn_acpi_madt_inventory_initialize(DAWN_ACPI_MADT_INVENTORY *inventory) {
    uint32_t index;

    inventory->physical_address = 0U;
    inventory->byte_size = 0U;
    inventory->local_interrupt_controller_address = 0U;
    inventory->flags = 0U;
    inventory->entry_count = 0U;
    inventory->omitted_entry_count = 0U;
    for (index = 0U; index < DAWN_ACPI_MADT_ENTRY_CAPACITY; ++index) {
        inventory->entries[index].physical_address = 0U;
        inventory->entries[index].byte_size = 0U;
        inventory->entries[index].type = 0U;
        inventory->entries[index].length = 0U;
    }
}

static int dawn_acpi_reader_checksum_is_zero(
    uint64_t physical_address, uint32_t byte_size, DAWN_ACPI_PHYSICAL_READER reader, void *reader_context) {
    uint8_t bytes[64];
    uint8_t checksum = 0U;
    uint32_t remaining = byte_size;
    uint32_t offset = 0U;
    uint32_t chunk_size;
    uint32_t index;

    while (remaining != 0U) {
        chunk_size = remaining > sizeof(bytes) ? (uint32_t)sizeof(bytes) : remaining;
        if (physical_address + offset < physical_address ||
            !reader(physical_address + offset, chunk_size, bytes, reader_context)) {
            return 0;
        }
        for (index = 0U; index < chunk_size; ++index) {
            checksum = (uint8_t)(checksum + bytes[index]);
        }
        offset += chunk_size;
        remaining -= chunk_size;
    }
    return checksum == 0U;
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

int dawn_acpi_child_table_inventory(
    const DAWN_ACPI_ROOT_TABLE_METADATA *root,
    DAWN_ACPI_PHYSICAL_READER reader,
    void *reader_context,
    DAWN_ACPI_CHILD_TABLE_INVENTORY *inventory) {
    uint8_t entry_bytes[8];
    uint8_t header[DAWN_ACPI_TABLE_HEADER_LENGTH];
    uint32_t entry_width;
    uint32_t retained_count;
    uint32_t entry_index;
    uint32_t header_length;
    uint32_t signature_index;
    uint64_t entry_address;
    uint64_t child_address;
    DAWN_ACPI_CHILD_TABLE_METADATA *entry;

    if (root == (const void *)0 || reader == (void *)0 || inventory == (void *)0 ||
        root->byte_size < DAWN_ACPI_TABLE_HEADER_LENGTH || root->entry_count == 0U ||
        (root->kind != DAWN_ACPI_ROOT_TABLE_RSDT && root->kind != DAWN_ACPI_ROOT_TABLE_XSDT)) {
        return 0;
    }
    entry_width = root->kind == DAWN_ACPI_ROOT_TABLE_XSDT ? 8U : 4U;
    if (((root->byte_size - DAWN_ACPI_TABLE_HEADER_LENGTH) / entry_width) != root->entry_count) {
        return 0;
    }
    dawn_acpi_child_inventory_initialize(inventory);
    retained_count = root->entry_count > DAWN_ACPI_CHILD_TABLE_CAPACITY ?
                         DAWN_ACPI_CHILD_TABLE_CAPACITY : root->entry_count;
    inventory->omitted_entry_count = root->entry_count - retained_count;
    for (entry_index = 0U; entry_index < retained_count; ++entry_index) {
        entry_address = root->physical_address + DAWN_ACPI_TABLE_HEADER_LENGTH + ((uint64_t)entry_index * entry_width);
        if (entry_address < root->physical_address || !reader(entry_address, entry_width, entry_bytes, reader_context)) {
            return 0;
        }
        child_address = entry_width == 8U ? dawn_acpi_read_u64_le(entry_bytes) : dawn_acpi_read_u32_le(entry_bytes);
        entry = &inventory->entries[entry_index];
        entry->physical_address = child_address;
        ++inventory->entry_count;
        if (child_address == 0U || !reader(child_address, sizeof(header), header, reader_context)) {
            continue;
        }
        header_length = dawn_acpi_read_u32_le(&header[4]);
        if (header_length < DAWN_ACPI_TABLE_HEADER_LENGTH || header_length > DAWN_ACPI_ROOT_TABLE_MAXIMUM_LENGTH) {
            continue;
        }
        for (signature_index = 0U; signature_index < 4U; ++signature_index) {
            entry->signature[signature_index] = header[signature_index];
        }
        entry->byte_size = header_length;
        entry->revision = header[8];
        entry->status = DAWN_ACPI_CHILD_TABLE_HEADER_VALID;
        if (dawn_acpi_reader_checksum_is_zero(child_address, header_length, reader, reader_context)) {
            entry->status |= DAWN_ACPI_CHILD_TABLE_CHECKSUM_VALID;
        }
    }
    return 1;
}

int dawn_acpi_madt_inventory(
    uint64_t physical_address,
    DAWN_ACPI_PHYSICAL_READER reader,
    void *reader_context,
    DAWN_ACPI_MADT_INVENTORY *inventory) {
    static const uint8_t signature[4] = {'A', 'P', 'I', 'C'};
    uint8_t header[DAWN_ACPI_MADT_FIXED_LENGTH];
    uint8_t entry_header[2];
    uint32_t byte_size;
    uint32_t offset;
    uint32_t index;
    uint64_t entry_address;

    if (inventory == (void *)0 || reader == (void *)0 || physical_address == 0U) {
        return 0;
    }
    dawn_acpi_madt_inventory_initialize(inventory);
    if (!reader(physical_address, sizeof(header), header, reader_context)) {
        return 0;
    }
    for (index = 0U; index < sizeof(signature); ++index) {
        if (header[index] != signature[index]) {
            return 0;
        }
    }
    byte_size = dawn_acpi_read_u32_le(&header[4]);
    if (byte_size < DAWN_ACPI_MADT_FIXED_LENGTH || byte_size > DAWN_ACPI_ROOT_TABLE_MAXIMUM_LENGTH ||
        !dawn_acpi_reader_checksum_is_zero(physical_address, byte_size, reader, reader_context)) {
        return 0;
    }
    inventory->physical_address = physical_address;
    inventory->byte_size = byte_size;
    inventory->local_interrupt_controller_address = dawn_acpi_read_u32_le(&header[36]);
    inventory->flags = dawn_acpi_read_u32_le(&header[40]);
    offset = DAWN_ACPI_MADT_FIXED_LENGTH;
    while (offset < byte_size) {
        entry_address = physical_address + offset;
        if (entry_address < physical_address || byte_size - offset < sizeof(entry_header) ||
            !reader(entry_address, sizeof(entry_header), entry_header, reader_context) || entry_header[1] < sizeof(entry_header) ||
            entry_header[1] > byte_size - offset) {
            return 0;
        }
        if (inventory->entry_count < DAWN_ACPI_MADT_ENTRY_CAPACITY) {
            DAWN_ACPI_MADT_ENTRY_METADATA *entry = &inventory->entries[inventory->entry_count];
            entry->physical_address = entry_address;
            entry->byte_size = entry_header[1];
            entry->type = entry_header[0];
            entry->length = entry_header[1];
            ++inventory->entry_count;
        } else {
            ++inventory->omitted_entry_count;
        }
        offset += entry_header[1];
    }
    return 1;
}
