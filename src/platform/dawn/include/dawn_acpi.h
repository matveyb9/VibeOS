/*
 * VibeOS Dawn ACPI metadata — a bounded RSDP integrity check at the
 * Prelude-to-Pulse boundary. This is deliberately not an ACPI table parser.
 */

#ifndef VIBEOS_DAWN_ACPI_H
#define VIBEOS_DAWN_ACPI_H

#include <stdint.h>

#define DAWN_ACPI_RSDP_V1_LENGTH UINT32_C(20)
#define DAWN_ACPI_RSDP_V2_MINIMUM_LENGTH UINT32_C(36)
#define DAWN_ACPI_TABLE_HEADER_LENGTH UINT32_C(36)
#define DAWN_ACPI_ROOT_TABLE_MAXIMUM_LENGTH UINT32_C(65536)
#define DAWN_ACPI_CHILD_TABLE_CAPACITY UINT32_C(64)

#define DAWN_ACPI_CHILD_TABLE_HEADER_VALID UINT8_C(0x01)
#define DAWN_ACPI_CHILD_TABLE_CHECKSUM_VALID UINT8_C(0x02)

typedef enum {
    DAWN_ACPI_ROOT_TABLE_RSDT = 1,
    DAWN_ACPI_ROOT_TABLE_XSDT = 2
} DAWN_ACPI_ROOT_TABLE_KIND;

typedef struct {
    uint64_t physical_address;
    uint32_t byte_size;
    uint32_t entry_count;
    uint8_t kind;
} DAWN_ACPI_ROOT_TABLE_METADATA;

typedef int (*DAWN_ACPI_PHYSICAL_READER)(
    uint64_t physical_address, uint32_t byte_count, uint8_t *destination, void *context);

typedef struct {
    uint64_t physical_address;
    uint32_t byte_size;
    uint8_t signature[4];
    uint8_t revision;
    uint8_t status;
} DAWN_ACPI_CHILD_TABLE_METADATA;

typedef struct {
    DAWN_ACPI_CHILD_TABLE_METADATA entries[DAWN_ACPI_CHILD_TABLE_CAPACITY];
    uint32_t entry_count;
    uint32_t omitted_entry_count;
} DAWN_ACPI_CHILD_TABLE_INVENTORY;

int dawn_acpi_rsdp_is_valid(const void *physical_rsdp);
int dawn_acpi_root_table_describe(const void *physical_rsdp, DAWN_ACPI_ROOT_TABLE_METADATA *metadata);
int dawn_acpi_child_table_inventory(
    const DAWN_ACPI_ROOT_TABLE_METADATA *root,
    DAWN_ACPI_PHYSICAL_READER reader,
    void *reader_context,
    DAWN_ACPI_CHILD_TABLE_INVENTORY *inventory);

#endif
