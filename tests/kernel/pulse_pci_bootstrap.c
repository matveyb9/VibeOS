#include <atlas_pci.h>
#include <stdint.h>
#include <stdio.h>

static uint32_t fake_read(void *context, uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    (void)context;
    if (bus != 0U) return UINT32_C(0xffffffff);
    if (device == 1U && function == 0U && offset == 0U) return UINT32_C(0x100e1234);
    if (device == 1U && function == 0U && offset == 8U) return UINT32_C(0x02000001);
    if (device == 1U && function == 0U && offset == 12U) return UINT32_C(0x00000000);
    if (device == 2U && function == 0U && offset == 0U) return UINT32_C(0x56781234);
    if (device == 2U && function == 0U && offset == 8U) return UINT32_C(0x06040002);
    if (device == 2U && function == 0U && offset == 12U) return UINT32_C(0x00800000);
    if (device == 2U && function == 3U && offset == 0U) return UINT32_C(0xabcd4321);
    if (device == 2U && function == 3U && offset == 8U) return UINT32_C(0x0c030003);
    if (device == 2U && function == 3U && offset == 12U) return UINT32_C(0x00000000);
    return UINT32_C(0xffffffff);
}

int main(void) {
    ATLAS_PCI_FUNCTION functions[4];
    uint32_t count = 0;
    if (!atlas_pci_inventory_scan(fake_read, (void *)0, functions, 4U, &count) || count != 3U ||
        functions[0].vendor_id != UINT16_C(0x1234) || functions[0].device_id != UINT16_C(0x100e) ||
        functions[0].class_code != 2U || functions[1].header_type != UINT8_C(0x80) ||
        functions[2].function != 3U || functions[2].class_code != UINT8_C(0x0c) ||
        atlas_pci_inventory_scan(fake_read, (void *)0, functions, 2U, &count)) {
        fputs("Atlas PCI bootstrap unit test failed.\n", stderr);
        return 1;
    }
    puts("Atlas PCI bootstrap unit tests passed.");
    return 0;
}
