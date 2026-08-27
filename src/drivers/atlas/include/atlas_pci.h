#ifndef VIBEOS_ATLAS_PCI_H
#define VIBEOS_ATLAS_PCI_H

#include <stdint.h>

#define ATLAS_PCI_INVENTORY_CAPACITY UINT32_C(32)

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
int atlas_pci_runtime_probe(void);

#endif
