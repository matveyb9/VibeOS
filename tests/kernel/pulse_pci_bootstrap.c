#include <atlas_pci.h>
#include <stdint.h>
#include <stdio.h>

static uint32_t fake_read(void *context, uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    (void)context;
    if (bus == 1U && device == 0U && function == 0U) {
        if (offset == 0U) return UINT32_C(0x29228086);
        if (offset == 8U) return UINT32_C(0x01060100);
        if (offset == 12U) return UINT32_C(0x00000000);
        return 0U;
    }
    if (bus == 0U && device == 1U && function == 0U) {
        if (offset == 0U) return UINT32_C(0x100e1234);
        if (offset == 8U) return UINT32_C(0x02000001);
        if (offset == 12U) return UINT32_C(0x00000000);
        if (offset == 16U) return UINT32_C(0x0000c001);
        if (offset == 20U) return UINT32_C(0x0000000c);
        if (offset == 24U) return UINT32_C(0x00000001);
        if (offset == 28U) return UINT32_C(0xfebf0000);
        if (offset == 32U) return UINT32_C(0x00000002);
        return 0U;
    }
    if (bus == 0U && device == 2U && function == 0U) {
        if (offset == 0U) return UINT32_C(0x56781234);
        if (offset == 8U) return UINT32_C(0x06040002);
        if (offset == 12U) return UINT32_C(0x00810000);
        if (offset == 24U) return UINT32_C(0x00010100);
        if (offset == 28U) return UINT32_C(0x00003020);
        if (offset == 32U) return UINT32_C(0x8ff08000);
        if (offset == 36U) return UINT32_C(0x9ff19001);
        if (offset == 40U) return UINT32_C(0x00000001);
        if (offset == 44U) return UINT32_C(0x00000002);
        return 0U;
    }
    if (bus == 0U && device == 2U && function == 3U) {
        if (offset == 0U) return UINT32_C(0xabcd4321);
        if (offset == 8U) return UINT32_C(0x0c030003);
        if (offset == 12U) return UINT32_C(0x00000000);
        return 0U;
    }
    return UINT32_C(0xffffffff);
}

int main(void) {
    ATLAS_PCI_FUNCTION functions[4];
    ATLAS_PCI_RESOURCE resources[24];
    uint32_t count = 0;
    if (!atlas_pci_inventory_scan(fake_read, (void *)0, functions, 4U, &count) || count != 3U ||
        functions[0].vendor_id != UINT16_C(0x1234) || functions[0].device_id != UINT16_C(0x100e) ||
        functions[0].class_code != 2U || functions[1].header_type != UINT8_C(0x81) ||
        functions[2].function != 3U || functions[2].class_code != UINT8_C(0x0c) ||
        !atlas_pci_inventory_scan_topology(fake_read, (void *)0, functions, 4U, &count) || count != 4U ||
        functions[3].bus != 1U || functions[3].vendor_id != UINT16_C(0x8086)) {
        fputs("Atlas PCI bootstrap inventory test failed.\n", stderr);
        return 1;
    }
    if (!atlas_pci_resource_inventory_scan(fake_read, (void *)0, functions, count, resources, 24U, &count) ||
        count != 21U || resources[0].kind != ATLAS_PCI_RESOURCE_BAR_IO ||
        resources[0].observed_base != UINT64_C(0xc000) ||
        resources[1].kind != ATLAS_PCI_RESOURCE_BAR_MEMORY64 ||
        resources[1].resource_index != 1U || resources[1].observed_base != UINT64_C(0x100000000) ||
        (resources[1].flags & (ATLAS_PCI_RESOURCE_ASSIGNED | ATLAS_PCI_RESOURCE_PREFETCHABLE |
                               ATLAS_PCI_RESOURCE_64BIT)) !=
            (ATLAS_PCI_RESOURCE_ASSIGNED | ATLAS_PCI_RESOURCE_PREFETCHABLE | ATLAS_PCI_RESOURCE_64BIT) ||
        resources[2].kind != ATLAS_PCI_RESOURCE_BAR_MEMORY32 ||
        resources[3].observed_base != 0U || resources[3].flags != 0U ||
        resources[6].kind != ATLAS_PCI_RESOURCE_BRIDGE_IO_APERTURE ||
        resources[6].observed_base != UINT64_C(0x2000) || resources[6].observed_limit != UINT64_C(0x3fff) ||
        resources[7].kind != ATLAS_PCI_RESOURCE_BRIDGE_MEMORY_APERTURE ||
        resources[7].observed_base != UINT64_C(0x80000000) || resources[7].observed_limit != UINT64_C(0x8fffffff) ||
        resources[8].kind != ATLAS_PCI_RESOURCE_BRIDGE_PREFETCHABLE_APERTURE ||
        resources[8].observed_base != UINT64_C(0x190000000) || resources[8].observed_limit != UINT64_C(0x29fffffff) ||
        atlas_pci_resource_inventory_scan(fake_read, (void *)0, functions, 4U, resources, 20U, &count) ||
        atlas_pci_inventory_scan(fake_read, (void *)0, functions, 2U, &count)) {
        fputs("Atlas PCI bootstrap unit test failed.\n", stderr);
        return 1;
    }
    puts("Atlas PCI bootstrap unit tests passed.");
    return 0;
}
