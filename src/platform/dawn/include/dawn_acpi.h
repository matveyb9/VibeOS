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

int dawn_acpi_rsdp_is_valid(const void *physical_rsdp);
int dawn_acpi_root_table_describe(const void *physical_rsdp, DAWN_ACPI_ROOT_TABLE_METADATA *metadata);

#endif
