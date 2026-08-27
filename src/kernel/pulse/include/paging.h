/* VibeOS Pulse — x86_64 early identity-paging boundary. */

#ifndef VIBEOS_PULSE_PAGING_H
#define VIBEOS_PULSE_PAGING_H

#include <stdint.h>

#define PULSE_X86_PAGE_SIZE UINT64_C(4096)
#define PULSE_X86_IDENTITY_MAP_BYTES UINT64_C(0x100000000)
#define PULSE_X86_IDENTITY_PAGE_DIRECTORIES UINT64_C(4)

typedef struct {
    uint64_t pml4_physical_address;
    uint64_t mapped_bytes;
} PULSE_PAGING_STATE;

int pulse_paging_build_identity_4g(
    uint64_t *pml4,
    uint64_t *pdpt,
    uint64_t *page_directories[PULSE_X86_IDENTITY_PAGE_DIRECTORIES],
    uint64_t pml4_physical_address,
    uint64_t pdpt_physical_address,
    const uint64_t page_directory_physical_addresses[PULSE_X86_IDENTITY_PAGE_DIRECTORIES]);
int pulse_paging_initialize(void);
const PULSE_PAGING_STATE *pulse_paging_state(void);

#endif
