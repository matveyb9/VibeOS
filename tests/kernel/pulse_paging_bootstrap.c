/* VibeOS Pulse — host checks for 4 GiB identity paging invariants. */

#include <stdint.h>
#include <stdio.h>

#include "paging.h"

#define TEST_PAGE_PRESENT UINT64_C(0x001)
#define TEST_PAGE_WRITABLE UINT64_C(0x002)
#define TEST_PAGE_LARGE UINT64_C(0x080)

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    uint64_t pml4[512];
    uint64_t pdpt[512];
    uint64_t directory_zero[512];
    uint64_t directory_one[512];
    uint64_t directory_two[512];
    uint64_t directory_three[512];
    uint64_t *page_directories[PULSE_X86_IDENTITY_PAGE_DIRECTORIES] = {
        directory_zero,
        directory_one,
        directory_two,
        directory_three,
    };
    const uint64_t pml4_physical_address = UINT64_C(0x300000);
    const uint64_t pdpt_physical_address = UINT64_C(0x301000);
    const uint64_t page_directory_physical_addresses[PULSE_X86_IDENTITY_PAGE_DIRECTORIES] = {
        UINT64_C(0x302000),
        UINT64_C(0x303000),
        UINT64_C(0x304000),
        UINT64_C(0x305000),
    };

    if (!expect(!pulse_paging_build_identity_4g(
                    pml4,
                    pdpt,
                    page_directories,
                    pml4_physical_address + 1U,
                    pdpt_physical_address,
                    page_directory_physical_addresses),
                "unaligned page-table address is rejected") ||
        !expect(pulse_paging_build_identity_4g(
                    pml4,
                    pdpt,
                    page_directories,
                    pml4_physical_address,
                    pdpt_physical_address,
                    page_directory_physical_addresses),
                "aligned page tables initialize")) {
        return 1;
    }

    if (!expect(pml4[0] == (pdpt_physical_address | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE),
                    "PML4 links to PDPT") ||
        !expect(pdpt[0] == (page_directory_physical_addresses[0] | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE) &&
                    pdpt[3] == (page_directory_physical_addresses[3] | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE),
                    "PDPT links all four page directories") ||
        !expect(directory_zero[0] == (TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE | TEST_PAGE_LARGE),
                    "first large page maps address zero") ||
        !expect(directory_three[511] ==
                    (UINT64_C(0xffe00000) | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE | TEST_PAGE_LARGE),
                    "last large page completes first four GiB") ||
        !expect(pml4[1] == 0U && pdpt[4] == 0U, "unused entries remain clear")) {
        return 1;
    }

    puts("Pulse paging bootstrap unit tests passed.");
    return 0;
}
