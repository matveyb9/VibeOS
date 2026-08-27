#ifndef VIBEOS_ATLAS_PCI_H
#define VIBEOS_ATLAS_PCI_H

#include <stdint.h>

#define ATLAS_PCI_INVENTORY_CAPACITY UINT32_C(32)
#define ATLAS_PCI_RESOURCE_CAPACITY (ATLAS_PCI_INVENTORY_CAPACITY * UINT32_C(6))

typedef uint32_t (*ATLAS_PCI_READ32)(void *context, uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t programming_interface;
    uint8_t revision;
    uint8_t header_type;
    uint16_t vendor_id;
    uint16_t device_id;
} ATLAS_PCI_FUNCTION;

typedef enum {
    ATLAS_PCI_RESOURCE_BAR_IO = 1,
    ATLAS_PCI_RESOURCE_BAR_MEMORY32 = 2,
    ATLAS_PCI_RESOURCE_BAR_MEMORY64 = 3,
    ATLAS_PCI_RESOURCE_BRIDGE_IO_APERTURE = 4,
    ATLAS_PCI_RESOURCE_BRIDGE_MEMORY_APERTURE = 5,
    ATLAS_PCI_RESOURCE_BRIDGE_PREFETCHABLE_APERTURE = 6
} ATLAS_PCI_RESOURCE_KIND;

enum {
    ATLAS_PCI_RESOURCE_ASSIGNED = UINT8_C(0x01),
    ATLAS_PCI_RESOURCE_PREFETCHABLE = UINT8_C(0x02),
    ATLAS_PCI_RESOURCE_64BIT = UINT8_C(0x04),
    ATLAS_PCI_RESOURCE_APERTURE = UINT8_C(0x08)
};

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t resource_index;
    uint8_t kind;
    uint8_t flags;
    uint16_t reserved;
    uint64_t observed_base;
    uint64_t observed_limit;
} ATLAS_PCI_RESOURCE;

int atlas_pci_inventory_scan(
    ATLAS_PCI_READ32 read32,
    void *context,
    ATLAS_PCI_FUNCTION *functions,
    uint32_t capacity,
    uint32_t *function_count);
int atlas_pci_inventory_scan_topology(
    ATLAS_PCI_READ32 read32,
    void *context,
    ATLAS_PCI_FUNCTION *functions,
    uint32_t capacity,
    uint32_t *function_count);
int atlas_pci_resource_inventory_scan(
    ATLAS_PCI_READ32 read32,
    void *context,
    const ATLAS_PCI_FUNCTION *functions,
    uint32_t function_count,
    ATLAS_PCI_RESOURCE *resources,
    uint32_t capacity,
    uint32_t *resource_count);
int atlas_pci_runtime_probe(void);

#endif
