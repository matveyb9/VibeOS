#include "atlas_pci.h"

#define ATLAS_PCI_VENDOR_ABSENT UINT16_C(0xffff)

static int atlas_pci_append(
    ATLAS_PCI_READ32 read32, void *context, uint8_t device, uint8_t function,
    ATLAS_PCI_FUNCTION *functions, uint32_t capacity, uint32_t *function_count) {
    uint32_t identity = read32(context, 0, device, function, 0);
    uint32_t class_word;
    uint32_t header_word;
    ATLAS_PCI_FUNCTION *entry;

    if ((uint16_t)identity == ATLAS_PCI_VENDOR_ABSENT) {
        return 1;
    }
    if (*function_count >= capacity) {
        return 0;
    }
    class_word = read32(context, 0, device, function, 8);
    header_word = read32(context, 0, device, function, 12);
    entry = &functions[*function_count];
    entry->bus = 0;
    entry->device = device;
    entry->function = function;
    entry->vendor_id = (uint16_t)identity;
    entry->device_id = (uint16_t)(identity >> 16);
    entry->revision = (uint8_t)class_word;
    entry->programming_interface = (uint8_t)(class_word >> 8);
    entry->subclass = (uint8_t)(class_word >> 16);
    entry->class_code = (uint8_t)(class_word >> 24);
    entry->header_type = (uint8_t)(header_word >> 16);
    ++*function_count;
    return 1;
}

int atlas_pci_inventory_scan(
    ATLAS_PCI_READ32 read32, void *context, ATLAS_PCI_FUNCTION *functions,
    uint32_t capacity, uint32_t *function_count) {
    uint32_t device;

    if (read32 == (void *)0 || functions == (void *)0 || function_count == (void *)0 || capacity == 0U) {
        return 0;
    }
    *function_count = 0;
    for (device = 0; device < 32U; ++device) {
        uint32_t before = *function_count;
        uint8_t function;

        if (!atlas_pci_append(read32, context, (uint8_t)device, 0, functions, capacity, function_count)) {
            return 0;
        }
        if (*function_count == before || (functions[before].header_type & 0x80U) == 0U) {
            continue;
        }
        for (function = 1; function < 8U; ++function) {
            if (!atlas_pci_append(read32, context, (uint8_t)device, function, functions, capacity, function_count)) {
                return 0;
            }
        }
    }
    return 1;
}

static uint32_t atlas_pci_x86_read32(void *context, uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = UINT32_C(0x80000000) | ((uint32_t)bus << 16) |
                       ((uint32_t)device << 11) | ((uint32_t)function << 8) | ((uint32_t)offset & 0xfcU);
    uint32_t value;

    (void)context;
    __asm__ volatile("outl %0, %w1" : : "a"(address), "d"((uint16_t)0xcf8) : "memory");
    __asm__ volatile("inl %w1, %0" : "=a"(value) : "d"((uint16_t)0xcfc) : "memory");
    return value;
}

int atlas_pci_runtime_probe(void) {
    ATLAS_PCI_FUNCTION functions[ATLAS_PCI_INVENTORY_CAPACITY];
    uint32_t function_count;

    return atlas_pci_inventory_scan(
        atlas_pci_x86_read32, (void *)0, functions, ATLAS_PCI_INVENTORY_CAPACITY, &function_count) &&
           function_count != 0U;
}
