/* VibeOS Dawn — bounded read-only x86 MADT metadata decoder. */

#include "dawn_acpi.h"

static uint32_t dawn_acpi_read_u32_le(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
           ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
}

static uint16_t dawn_acpi_read_u16_le(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8U));
}

static void dawn_acpi_madt_x86_initialize(DAWN_ACPI_MADT_X86_INVENTORY *inventory) {
    uint32_t index;

    inventory->local_apic_count = 0U;
    inventory->io_apic_count = 0U;
    inventory->interrupt_source_override_count = 0U;
    inventory->omitted_local_apic_count = 0U;
    inventory->omitted_io_apic_count = 0U;
    inventory->omitted_interrupt_source_override_count = 0U;
    for (index = 0U; index < DAWN_ACPI_MADT_X86_RECORD_CAPACITY; ++index) {
        inventory->local_apics[index].acpi_processor_id = 0U;
        inventory->local_apics[index].local_apic_id = 0U;
        inventory->local_apics[index].flags = 0U;
        inventory->io_apics[index].io_apic_id = 0U;
        inventory->io_apics[index].io_apic_physical_address = 0U;
        inventory->io_apics[index].global_system_interrupt_base = 0U;
        inventory->interrupt_source_overrides[index].bus = 0U;
        inventory->interrupt_source_overrides[index].source = 0U;
        inventory->interrupt_source_overrides[index].global_system_interrupt = 0U;
        inventory->interrupt_source_overrides[index].flags = 0U;
    }
}

int dawn_acpi_madt_x86_inventory(
    const DAWN_ACPI_MADT_INVENTORY *madt,
    DAWN_ACPI_PHYSICAL_READER reader,
    void *reader_context,
    DAWN_ACPI_MADT_X86_INVENTORY *inventory) {
    uint8_t bytes[12];
    uint32_t index;

    if (madt == (const void *)0 || reader == (void *)0 || inventory == (void *)0 ||
        madt->entry_count > DAWN_ACPI_MADT_ENTRY_CAPACITY) {
        return 0;
    }
    dawn_acpi_madt_x86_initialize(inventory);
    for (index = 0U; index < madt->entry_count; ++index) {
        const DAWN_ACPI_MADT_ENTRY_METADATA *entry = &madt->entries[index];

        if (entry->type == 0U) {
            if (entry->length != 8U || !reader(entry->physical_address, 8U, bytes, reader_context)) {
                return 0;
            }
            if (inventory->local_apic_count < DAWN_ACPI_MADT_X86_RECORD_CAPACITY) {
                DAWN_ACPI_MADT_LOCAL_APIC_METADATA *record = &inventory->local_apics[inventory->local_apic_count++];
                record->acpi_processor_id = bytes[2];
                record->local_apic_id = bytes[3];
                record->flags = dawn_acpi_read_u32_le(&bytes[4]);
            } else {
                ++inventory->omitted_local_apic_count;
            }
        } else if (entry->type == 1U) {
            if (entry->length != 12U || !reader(entry->physical_address, 12U, bytes, reader_context)) {
                return 0;
            }
            if (inventory->io_apic_count < DAWN_ACPI_MADT_X86_RECORD_CAPACITY) {
                DAWN_ACPI_MADT_IO_APIC_METADATA *record = &inventory->io_apics[inventory->io_apic_count++];
                record->io_apic_id = bytes[2];
                record->io_apic_physical_address = dawn_acpi_read_u32_le(&bytes[4]);
                record->global_system_interrupt_base = dawn_acpi_read_u32_le(&bytes[8]);
            } else {
                ++inventory->omitted_io_apic_count;
            }
        } else if (entry->type == 2U) {
            if (entry->length != 10U || !reader(entry->physical_address, 10U, bytes, reader_context)) {
                return 0;
            }
            if (inventory->interrupt_source_override_count < DAWN_ACPI_MADT_X86_RECORD_CAPACITY) {
                DAWN_ACPI_MADT_INTERRUPT_SOURCE_OVERRIDE_METADATA *record =
                    &inventory->interrupt_source_overrides[inventory->interrupt_source_override_count++];
                record->bus = bytes[2];
                record->source = bytes[3];
                record->global_system_interrupt = dawn_acpi_read_u32_le(&bytes[4]);
                record->flags = dawn_acpi_read_u16_le(&bytes[8]);
            } else {
                ++inventory->omitted_interrupt_source_override_count;
            }
        }
    }
    return 1;
}
