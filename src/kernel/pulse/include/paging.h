/* VibeOS Pulse — x86_64 early four-level paging contract. */

#ifndef VIBEOS_PULSE_PAGING_H
#define VIBEOS_PULSE_PAGING_H

#include <stdint.h>

#define PULSE_X86_PAGE_SIZE UINT64_C(4096)
#define PULSE_X86_IDENTITY_MAP_BYTES UINT64_C(0x40000000)

typedef struct {
    uint64_t pml4_physical_address;
    uint64_t mapped_bytes;
} PULSE_PAGING_STATE;

int pulse_paging_build_identity_1g(
    uint64_t *pml4,
    uint64_t *pdpt,
    uint64_t *page_directory,
    uint64_t pml4_physical_address,
    uint64_t pdpt_physical_address,
    uint64_t page_directory_physical_address);
int pulse_paging_initialize(void);
const PULSE_PAGING_STATE *pulse_paging_state(void);

#endif
