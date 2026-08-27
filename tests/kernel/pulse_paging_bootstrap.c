/* VibeOS Pulse — host checks for x86_64 early page-table layout. */

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
    uint64_t page_directory[512];
    const uint64_t pml4_physical_address = UINT64_C(0x300000);
    const uint64_t pdpt_physical_address = UINT64_C(0x301000);
    const uint64_t page_directory_physical_address = UINT64_C(0x302000);

    if (!expect(!pulse_paging_build_identity_1g(
                    pml4,
                    pdpt,
                    page_directory,
                    pml4_physical_address + 1U,
                    pdpt_physical_address,
                    page_directory_physical_address),
                "unaligned page-table address is rejected") ||
        !expect(pulse_paging_build_identity_1g(
                    pml4,
                    pdpt,
                    page_directory,
                    pml4_physical_address,
                    pdpt_physical_address,
                    page_directory_physical_address),
                "aligned page tables initialize")) {
        return 1;
    }

    if (!expect(pml4[0] == (pdpt_physical_address | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE),
                    "PML4 links to PDPT") ||
        !expect(pdpt[0] == (page_directory_physical_address | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE),
                    "PDPT links to page directory") ||
        !expect(page_directory[0] == (TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE | TEST_PAGE_LARGE),
                    "first large page maps address zero") ||
        !expect(page_directory[511] ==
                    (UINT64_C(0x3fe00000) | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE | TEST_PAGE_LARGE),
                    "last large page completes first GiB") ||
        !expect(pml4[1] == 0U && pdpt[1] == 0U, "unused entries remain clear")) {
        return 1;
    }

    puts("Pulse paging bootstrap unit tests passed.");
    return 0;
}
