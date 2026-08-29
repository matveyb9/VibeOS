/* VibeOS Atlas — bounded read-only ATA PIO IDENTIFY contract. */

#ifndef VIBEOS_ATLAS_ATA_H
#define VIBEOS_ATLAS_ATA_H

#include <stdint.h>

#define ATLAS_ATA_IDENTIFY_WORDS UINT32_C(256)
#define ATLAS_ATA_PIO_SPIN_LIMIT UINT32_C(1000000)
#define ATLAS_ATA_PRIMARY_COMMAND_BASE UINT16_C(0x1f0)
#define ATLAS_ATA_PRIMARY_CONTROL_BASE UINT16_C(0x3f6)

typedef uint8_t (*ATLAS_ATA_IN8)(void *context, uint16_t port);
typedef void (*ATLAS_ATA_OUT8)(void *context, uint16_t port, uint8_t value);
typedef uint16_t (*ATLAS_ATA_IN16)(void *context, uint16_t port);

typedef struct {
    ATLAS_ATA_IN8 in8;
    ATLAS_ATA_OUT8 out8;
    ATLAS_ATA_IN16 in16;
    void *context;
} ATLAS_ATA_PIO_TRANSPORT;

typedef struct {
    uint64_t logical_sector_count;
    uint8_t lba48_supported;
} ATLAS_ATA_IDENTIFY_INFO;

int atlas_ata_identify_parse(
    const uint16_t words[ATLAS_ATA_IDENTIFY_WORDS], ATLAS_ATA_IDENTIFY_INFO *info);
int atlas_ata_pio_identify(
    const ATLAS_ATA_PIO_TRANSPORT *transport,
    uint16_t command_base,
    uint16_t control_base,
    uint8_t device,
    ATLAS_ATA_IDENTIFY_INFO *info);
int atlas_ata_runtime_probe(void);

#endif
