/*
 * VibeOS Pulse — early x86_64 four-level identity mapping.
 *
 * The temporary map covers the lower 4 GiB in 2 MiB pages. It contains the
 * firmware framebuffer range used by the initial Prism software-renderer
 * bootstrap, before Pulse introduces a narrower virtual-memory layout.
 */

#include "memory.h"
#include "paging.h"

#define PULSE_X86_PAGE_PRESENT UINT64_C(0x001)
#define PULSE_X86_PAGE_WRITABLE UINT64_C(0x002)
#define PULSE_X86_PAGE_LARGE UINT64_C(0x080)
#define PULSE_X86_PAGE_ADDRESS_MASK UINT64_C(0x000ffffffffff000)
#define PULSE_X86_CR4_LA57 UINT64_C(0x1000)
#define PULSE_X86_PAGE_ENTRIES UINT64_C(512)
#define PULSE_X86_LARGE_PAGE_SIZE UINT64_C(0x200000)
#define PULSE_X86_GIBIBYTE UINT64_C(0x40000000)

static PULSE_PAGING_STATE early_paging_state;

static void pulse_zero_page(uint64_t *page) {
    uint64_t index;

    for (index = 0; index < PULSE_X86_PAGE_ENTRIES; ++index) {
        page[index] = 0;
    }
}

static int pulse_is_page_aligned(uint64_t address) {
    return (address & (PULSE_X86_PAGE_SIZE - 1U)) == 0U;
}

static uint64_t pulse_read_cr4(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr4, %0" : "=r"(value));
    return value;
}

static void pulse_write_cr3(uint64_t value) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(value) : "memory");
}

int pulse_paging_build_identity_4g(
    uint64_t *pml4,
    uint64_t *pdpt,
    uint64_t *page_directories[PULSE_X86_IDENTITY_PAGE_DIRECTORIES],
    uint64_t pml4_physical_address,
    uint64_t pdpt_physical_address,
    const uint64_t page_directory_physical_addresses[PULSE_X86_IDENTITY_PAGE_DIRECTORIES]) {
    uint64_t directory_index;
    uint64_t entry_index;

    if (pml4 == (void *)0 || pdpt == (void *)0 || page_directories == (void *)0 ||
        page_directory_physical_addresses == (void *)0 || !pulse_is_page_aligned(pml4_physical_address) ||
        !pulse_is_page_aligned(pdpt_physical_address)) {
        return 0;
    }
    for (directory_index = 0; directory_index < PULSE_X86_IDENTITY_PAGE_DIRECTORIES; ++directory_index) {
        if (page_directories[directory_index] == (void *)0 ||
            !pulse_is_page_aligned(page_directory_physical_addresses[directory_index])) {
            return 0;
        }
    }

    pulse_zero_page(pml4);
    pulse_zero_page(pdpt);
    pml4[0] = (pdpt_physical_address & PULSE_X86_PAGE_ADDRESS_MASK) |
              PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE;

    for (directory_index = 0; directory_index < PULSE_X86_IDENTITY_PAGE_DIRECTORIES; ++directory_index) {
        uint64_t *page_directory = page_directories[directory_index];

        pulse_zero_page(page_directory);
        pdpt[directory_index] =
            (page_directory_physical_addresses[directory_index] & PULSE_X86_PAGE_ADDRESS_MASK) |
            PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE;
        for (entry_index = 0; entry_index < PULSE_X86_PAGE_ENTRIES; ++entry_index) {
            page_directory[entry_index] = ((directory_index * PULSE_X86_GIBIBYTE) +
                                           (entry_index * PULSE_X86_LARGE_PAGE_SIZE)) |
                                          PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE |
                                          PULSE_X86_PAGE_LARGE;
        }
    }
    return 1;
}

int pulse_paging_initialize(void) {
    uint64_t pml4_physical_address;
    uint64_t pdpt_physical_address;
    uint64_t page_directory_physical_addresses[PULSE_X86_IDENTITY_PAGE_DIRECTORIES];
    uint64_t *page_directories[PULSE_X86_IDENTITY_PAGE_DIRECTORIES];
    uint64_t index;

    if ((pulse_read_cr4() & PULSE_X86_CR4_LA57) != 0U ||
        !pulse_memory_take_frame(&pml4_physical_address) ||
        !pulse_memory_take_frame(&pdpt_physical_address)) {
        return 0;
    }
    for (index = 0; index < PULSE_X86_IDENTITY_PAGE_DIRECTORIES; ++index) {
        if (!pulse_memory_take_frame(&page_directory_physical_addresses[index])) {
            return 0;
        }
        page_directories[index] = (uint64_t *)(uintptr_t)page_directory_physical_addresses[index];
    }
    if (!pulse_paging_build_identity_4g(
            (uint64_t *)(uintptr_t)pml4_physical_address,
            (uint64_t *)(uintptr_t)pdpt_physical_address,
            page_directories,
            pml4_physical_address,
            pdpt_physical_address,
            page_directory_physical_addresses)) {
        return 0;
    }

    pulse_write_cr3(pml4_physical_address);
    early_paging_state.pml4_physical_address = pml4_physical_address;
    early_paging_state.mapped_bytes = PULSE_X86_IDENTITY_MAP_BYTES;
    return 1;
}

const PULSE_PAGING_STATE *pulse_paging_state(void) {
    return &early_paging_state;
}
