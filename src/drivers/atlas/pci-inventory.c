#include "atlas_pci.h"

#define ATLAS_PCI_VENDOR_ABSENT UINT16_C(0xffff)

static int atlas_pci_append(ATLAS_PCI_READ32 read32, void *context, uint8_t bus, uint8_t device, uint8_t function, ATLAS_PCI_FUNCTION *functions, uint32_t capacity, uint32_t *count) {
    uint32_t identity = read32(context, bus, device, function, 0);
    uint32_t class_word;
    uint32_t header_word;
    ATLAS_PCI_FUNCTION *entry;
    if ((uint16_t)identity == ATLAS_PCI_VENDOR_ABSENT) return 1;
    if (*count >= capacity) return 0;
    class_word = read32(context, bus, device, function, 8);
    header_word = read32(context, bus, device, function, 12);
    entry = &functions[*count];
    entry->bus = bus; entry->device = device; entry->function = function;
    entry->vendor_id = (uint16_t)identity; entry->device_id = (uint16_t)(identity >> 16);
    entry->revision = (uint8_t)class_word; entry->programming_interface = (uint8_t)(class_word >> 8);
    entry->subclass = (uint8_t)(class_word >> 16); entry->class_code = (uint8_t)(class_word >> 24);
    entry->header_type = (uint8_t)(header_word >> 16);
    ++*count;
    return 1;
}

static int atlas_pci_scan_bus(ATLAS_PCI_READ32 read32, void *context, ATLAS_PCI_FUNCTION *functions, uint32_t capacity, uint32_t *count, uint8_t bus) {
    uint32_t device;
    for (device = 0; device < 32U; ++device) {
        uint32_t before = *count;
        uint8_t function;
        if (!atlas_pci_append(read32, context, bus, (uint8_t)device, 0, functions, capacity, count)) return 0;
        if (*count == before || (functions[before].header_type & 0x80U) == 0U) continue;
        for (function = 1; function < 8U; ++function)
            if (!atlas_pci_append(read32, context, bus, (uint8_t)device, function, functions, capacity, count)) return 0;
    }
    return 1;
}

int atlas_pci_inventory_scan(ATLAS_PCI_READ32 read32, void *context, ATLAS_PCI_FUNCTION *functions, uint32_t capacity, uint32_t *count) {
    if (read32 == (void *)0 || functions == (void *)0 || count == (void *)0 || capacity == 0U) return 0;
    *count = 0;
    return atlas_pci_scan_bus(read32, context, functions, capacity, count, 0);
}

int atlas_pci_inventory_scan_topology(ATLAS_PCI_READ32 read32, void *context, ATLAS_PCI_FUNCTION *functions, uint32_t capacity, uint32_t *count) {
    uint8_t buses[ATLAS_PCI_INVENTORY_CAPACITY];
    uint8_t visited[256];
    uint32_t head = 0; uint32_t tail = 1;
    uint32_t bus_index;
    if (read32 == (void *)0 || functions == (void *)0 || count == (void *)0 || capacity == 0U) return 0;
    for (bus_index = 0; bus_index < 256U; ++bus_index) visited[bus_index] = 0;
    *count = 0; buses[0] = 0;
    while (head < tail) {
        uint8_t bus = buses[head++]; uint32_t first = *count; uint32_t index;
        if (visited[bus]) continue;
        visited[bus] = 1;
        if (!atlas_pci_scan_bus(read32, context, functions, capacity, count, bus)) return 0;
        for (index = first; index < *count; ++index) {
            ATLAS_PCI_FUNCTION *entry = &functions[index];
            uint8_t secondary;
            if (entry->class_code != 6U || entry->subclass != 4U || (entry->header_type & 0x7fU) != 1U) continue;
            secondary = (uint8_t)(read32(context, entry->bus, entry->device, entry->function, 24) >> 8);
            if (secondary != 0U && !visited[secondary]) {
                if (tail >= ATLAS_PCI_INVENTORY_CAPACITY) return 0;
                buses[tail++] = secondary;
            }
        }
    }
    return 1;
}

static int atlas_pci_resource_append(
    const ATLAS_PCI_FUNCTION *function,
    uint8_t resource_index,
    uint8_t kind,
    uint8_t flags,
    uint64_t observed_base,
    uint64_t observed_limit,
    ATLAS_PCI_RESOURCE *resources,
    uint32_t capacity,
    uint32_t *count) {
    ATLAS_PCI_RESOURCE *entry;

    if (*count >= capacity) return 0;
    entry = &resources[*count];
    entry->bus = function->bus;
    entry->device = function->device;
    entry->function = function->function;
    entry->resource_index = resource_index;
    entry->kind = kind;
    entry->flags = flags;
    entry->reserved = 0U;
    entry->observed_base = observed_base;
    entry->observed_limit = observed_limit;
    ++*count;
    return 1;
}

static int atlas_pci_append_bars(
    ATLAS_PCI_READ32 read32,
    void *context,
    const ATLAS_PCI_FUNCTION *function,
    ATLAS_PCI_RESOURCE *resources,
    uint32_t capacity,
    uint32_t *count) {
    uint32_t bar_count;
    uint32_t bar_index;

    if ((function->header_type & UINT8_C(0x7f)) == 0U) bar_count = 6U;
    else if ((function->header_type & UINT8_C(0x7f)) == 1U) bar_count = 2U;
    else return 1;
    for (bar_index = 0U; bar_index < bar_count; ++bar_index) {
        uint32_t resource_index = bar_index;
        uint32_t lower = read32(context, function->bus, function->device, function->function,
                                (uint8_t)(UINT8_C(0x10) + (bar_index * 4U)));
        uint8_t kind;
        uint8_t flags = 0U;
        uint64_t observed_base;

        if ((lower & UINT32_C(0x1)) != 0U) {
            kind = ATLAS_PCI_RESOURCE_BAR_IO;
            observed_base = (uint64_t)(lower & UINT32_C(0xfffffffc));
        } else {
            uint32_t memory_type = (lower >> 1U) & UINT32_C(0x3);

            if (memory_type == 0U) {
                kind = ATLAS_PCI_RESOURCE_BAR_MEMORY32;
                observed_base = (uint64_t)(lower & UINT32_C(0xfffffff0));
            } else if (memory_type == 2U && bar_index + 1U < bar_count) {
                uint32_t upper = read32(context, function->bus, function->device, function->function,
                                         (uint8_t)(UINT8_C(0x14) + (bar_index * 4U)));

                kind = ATLAS_PCI_RESOURCE_BAR_MEMORY64;
                flags = ATLAS_PCI_RESOURCE_64BIT;
                observed_base = ((uint64_t)upper << 32U) | (uint64_t)(lower & UINT32_C(0xfffffff0));
                ++bar_index;
            } else {
                continue;
            }
            if ((lower & UINT32_C(0x8)) != 0U) flags |= ATLAS_PCI_RESOURCE_PREFETCHABLE;
        }
        if (observed_base != 0U) flags |= ATLAS_PCI_RESOURCE_ASSIGNED;
        if (!atlas_pci_resource_append(function, (uint8_t)resource_index, kind, flags, observed_base, 0U,
                                       resources, capacity, count)) return 0;
    }
    return 1;
}

static int atlas_pci_append_bridge_apertures(
    ATLAS_PCI_READ32 read32,
    void *context,
    const ATLAS_PCI_FUNCTION *function,
    ATLAS_PCI_RESOURCE *resources,
    uint32_t capacity,
    uint32_t *count) {
    uint32_t io_word;
    uint32_t io_upper;
    uint32_t memory_word;
    uint32_t prefetch_word;
    uint32_t prefetch_base_upper;
    uint32_t prefetch_limit_upper;
    uint8_t io_base_type;
    uint8_t io_limit_type;
    uint64_t observed_base;
    uint64_t observed_limit;

    if (function->class_code != 6U || function->subclass != 4U ||
        (function->header_type & UINT8_C(0x7f)) != 1U) return 1;
    io_word = read32(context, function->bus, function->device, function->function, UINT8_C(0x1c));
    io_upper = read32(context, function->bus, function->device, function->function, UINT8_C(0x30));
    io_base_type = (uint8_t)(io_word & UINT32_C(0x0f));
    io_limit_type = (uint8_t)((io_word >> 8U) & UINT32_C(0x0f));
    if (io_base_type == io_limit_type && (io_base_type == 0U || io_base_type == 1U)) {
        observed_base = (uint64_t)((io_word & UINT32_C(0x000000f0)) << 8U);
        observed_limit = (uint64_t)(io_word & UINT32_C(0x0000f000)) | UINT64_C(0xfff);
        if (io_base_type == 1U) {
            observed_base |= (uint64_t)(uint16_t)io_upper << 16U;
            observed_limit |= (uint64_t)(uint16_t)(io_upper >> 16U) << 16U;
        }
        if (observed_base <= observed_limit &&
            !atlas_pci_resource_append(function, 0U, ATLAS_PCI_RESOURCE_BRIDGE_IO_APERTURE,
                                       ATLAS_PCI_RESOURCE_ASSIGNED | ATLAS_PCI_RESOURCE_APERTURE,
                                       observed_base, observed_limit, resources, capacity, count)) return 0;
    }
    memory_word = read32(context, function->bus, function->device, function->function, UINT8_C(0x20));
    observed_base = (uint64_t)((memory_word & UINT32_C(0x0000fff0)) << 16U);
    observed_limit = (uint64_t)(memory_word & UINT32_C(0xfff00000)) | UINT64_C(0xfffff);
    if (observed_base <= observed_limit &&
        !atlas_pci_resource_append(function, 1U, ATLAS_PCI_RESOURCE_BRIDGE_MEMORY_APERTURE,
                                   ATLAS_PCI_RESOURCE_ASSIGNED | ATLAS_PCI_RESOURCE_APERTURE,
                                   observed_base, observed_limit, resources, capacity, count)) return 0;
    prefetch_word = read32(context, function->bus, function->device, function->function, UINT8_C(0x24));
    if ((prefetch_word & UINT32_C(0x0f)) == ((prefetch_word >> 16U) & UINT32_C(0x0f)) &&
        ((prefetch_word & UINT32_C(0x0f)) == 0U || (prefetch_word & UINT32_C(0x0f)) == 1U)) {
        observed_base = (uint64_t)((prefetch_word & UINT32_C(0x0000fff0)) << 16U);
        observed_limit = (uint64_t)(prefetch_word & UINT32_C(0xfff00000)) | UINT64_C(0xfffff);
        if ((prefetch_word & UINT32_C(0x0f)) == 1U) {
            prefetch_base_upper = read32(context, function->bus, function->device, function->function, UINT8_C(0x28));
            prefetch_limit_upper = read32(context, function->bus, function->device, function->function, UINT8_C(0x2c));
            observed_base |= (uint64_t)prefetch_base_upper << 32U;
            observed_limit |= (uint64_t)prefetch_limit_upper << 32U;
        }
        if (observed_base <= observed_limit &&
            !atlas_pci_resource_append(function, 2U, ATLAS_PCI_RESOURCE_BRIDGE_PREFETCHABLE_APERTURE,
                                       ATLAS_PCI_RESOURCE_ASSIGNED | ATLAS_PCI_RESOURCE_PREFETCHABLE |
                                           ATLAS_PCI_RESOURCE_APERTURE,
                                       observed_base, observed_limit, resources, capacity, count)) return 0;
    }
    return 1;
}

int atlas_pci_resource_inventory_scan(
    ATLAS_PCI_READ32 read32,
    void *context,
    const ATLAS_PCI_FUNCTION *functions,
    uint32_t function_count,
    ATLAS_PCI_RESOURCE *resources,
    uint32_t capacity,
    uint32_t *resource_count) {
    uint32_t function_index;

    if (read32 == (void *)0 || functions == (void *)0 || function_count == 0U ||
        resources == (void *)0 || resource_count == (void *)0 || capacity == 0U) return 0;
    *resource_count = 0U;
    for (function_index = 0U; function_index < function_count; ++function_index) {
        if (!atlas_pci_append_bars(read32, context, &functions[function_index], resources, capacity, resource_count) ||
            !atlas_pci_append_bridge_apertures(read32, context, &functions[function_index], resources,
                                               capacity, resource_count)) return 0;
    }
    return 1;
}

static uint32_t atlas_pci_x86_read32(void *context, uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = UINT32_C(0x80000000) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | ((uint32_t)function << 8) | ((uint32_t)offset & 0xfcU), value;
    (void)context;
    __asm__ volatile("outl %0, %w1" : : "a"(address), "d"((uint16_t)0xcf8) : "memory");
    __asm__ volatile("inl %w1, %0" : "=a"(value) : "d"((uint16_t)0xcfc) : "memory");
    return value;
}

int atlas_pci_runtime_probe(void) {
    ATLAS_PCI_FUNCTION functions[ATLAS_PCI_INVENTORY_CAPACITY];
    ATLAS_PCI_RESOURCE resources[ATLAS_PCI_RESOURCE_CAPACITY];
    uint32_t function_count;
    uint32_t resource_count;

    return atlas_pci_inventory_scan_topology(atlas_pci_x86_read32, (void *)0, functions,
                                             ATLAS_PCI_INVENTORY_CAPACITY, &function_count) &&
           function_count != 0U &&
           atlas_pci_resource_inventory_scan(atlas_pci_x86_read32, (void *)0, functions, function_count,
                                             resources, ATLAS_PCI_RESOURCE_CAPACITY, &resource_count);
}
