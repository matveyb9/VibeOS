/*
 * VibeOS Pulse — early x86_64 four-level identity mapping.
 *
 * The map covers physical and virtual addresses from 0 through 1 GiB with
 * 2 MiB pages. It exists solely to take ownership of CR3 before higher-half
 * layout, W^X permissions, and fine-grained virtual-memory policy arrive.
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

int pulse_paging_build_identity_1g(
    uint64_t *pml4,
    uint64_t *pdpt,
    uint64_t *page_directory,
    uint64_t pml4_physical_address,
    uint64_t pdpt_physical_address,
    uint64_t page_directory_physical_address) {
    uint64_t index;

    if (pml4 == (void *)0 || pdpt == (void *)0 || page_directory == (void *)0 ||
        !pulse_is_page_aligned(pml4_physical_address) || !pulse_is_page_aligned(pdpt_physical_address) ||
        !pulse_is_page_aligned(page_directory_physical_address)) {
        return 0;
    }

    pulse_zero_page(pml4);
    pulse_zero_page(pdpt);
    pulse_zero_page(page_directory);

    pml4[0] = (pdpt_physical_address & PULSE_X86_PAGE_ADDRESS_MASK) |
              PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE;
    pdpt[0] = (page_directory_physical_address & PULSE_X86_PAGE_ADDRESS_MASK) |
              PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE;

    for (index = 0; index < PULSE_X86_PAGE_ENTRIES; ++index) {
        page_directory[index] = (index * PULSE_X86_LARGE_PAGE_SIZE) |
                                PULSE_X86_PAGE_PRESENT | PULSE_X86_PAGE_WRITABLE |
                                PULSE_X86_PAGE_LARGE;
    }

    return 1;
}

int pulse_paging_initialize(void) {
    uint64_t pml4_physical_address;
    uint64_t pdpt_physical_address;
    uint64_t page_directory_physical_address;

    if ((pulse_read_cr4() & PULSE_X86_CR4_LA57) != 0U ||
        !pulse_memory_take_frame(&pml4_physical_address) ||
        !pulse_memory_take_frame(&pdpt_physical_address) ||
        !pulse_memory_take_frame(&page_directory_physical_address) ||
        !pulse_paging_build_identity_1g(
            (uint64_t *)(uintptr_t)pml4_physical_address,
            (uint64_t *)(uintptr_t)pdpt_physical_address,
            (uint64_t *)(uintptr_t)page_directory_physical_address,
            pml4_physical_address,
            pdpt_physical_address,
            page_directory_physical_address)) {
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
