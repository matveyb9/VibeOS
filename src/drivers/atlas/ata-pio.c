/* VibeOS Atlas — x86 ATA PIO IDENTIFY only; no sector or write path. */

#include "atlas_ata.h"

#define ATLAS_ATA_REGISTER_DATA UINT16_C(0)
#define ATLAS_ATA_REGISTER_SECTOR_COUNT UINT16_C(2)
#define ATLAS_ATA_REGISTER_LBA_LOW UINT16_C(3)
#define ATLAS_ATA_REGISTER_LBA_MID UINT16_C(4)
#define ATLAS_ATA_REGISTER_LBA_HIGH UINT16_C(5)
#define ATLAS_ATA_REGISTER_DEVICE UINT16_C(6)
#define ATLAS_ATA_REGISTER_STATUS_COMMAND UINT16_C(7)
#define ATLAS_ATA_STATUS_ERR UINT8_C(0x01)
#define ATLAS_ATA_STATUS_DRQ UINT8_C(0x08)
#define ATLAS_ATA_STATUS_DF UINT8_C(0x20)
#define ATLAS_ATA_STATUS_BSY UINT8_C(0x80)
#define ATLAS_ATA_COMMAND_IDENTIFY_DEVICE UINT8_C(0xec)
#define ATLAS_ATA_COMMAND_READ_SECTORS UINT8_C(0x20)
#define ATLAS_ATA_STATUS_READY UINT8_C(0x40)
#define ATLAS_ATA_DEVICE_LBA_MODE UINT8_C(0x40)
#define ATLAS_ATA_DEVICE_SELECT_BASE UINT8_C(0xa0)
#define ATLAS_ATA_DEVICE_SELECT_SLAVE UINT8_C(0x10)
#define ATLAS_ATA_LBA48_SUPPORTED UINT16_C(0x0400)

static uint8_t atlas_ata_x86_in8(void *context, uint16_t port) {
    uint8_t value;

    (void)context;
    __asm__ volatile("inb %w1, %0" : "=a"(value) : "d"(port));
    return value;
}

static void atlas_ata_x86_out8(void *context, uint16_t port, uint8_t value) {
    (void)context;
    __asm__ volatile("outb %0, %w1" : : "a"(value), "d"(port));
}

static uint16_t atlas_ata_x86_in16(void *context, uint16_t port) {
    uint16_t value;

    (void)context;
    __asm__ volatile("inw %w1, %0" : "=a"(value) : "d"(port));
    return value;
}

int atlas_ata_device_handle_admit(
    uint16_t command_base,
    uint16_t control_base,
    uint8_t device,
    const ATLAS_ATA_IDENTIFY_INFO *info,
    uint64_t identity_fingerprint,
    ATLAS_ATA_DEVICE_HANDLE *handle) {
    if (info == (const void *)0 || handle == (void *)0 || device > 1U ||
        command_base > UINT16_MAX - ATLAS_ATA_REGISTER_STATUS_COMMAND ||
        control_base == UINT16_MAX || info->logical_sector_count == 0U ||
        identity_fingerprint == 0U) {
        return 0;
    }
    handle->command_base = command_base;
    handle->control_base = control_base;
    handle->device = device;
    handle->version = ATLAS_ATA_DEVICE_HANDLE_VERSION;
    handle->logical_sector_count = info->logical_sector_count;
    handle->identity_fingerprint = identity_fingerprint;
    return 1;
}

int atlas_ata_device_handle_matches(
    const ATLAS_ATA_DEVICE_HANDLE *handle,
    uint16_t command_base,
    uint16_t control_base,
    uint8_t device,
    const ATLAS_ATA_IDENTIFY_INFO *info,
    uint64_t identity_fingerprint) {
    return handle != (const void *)0 && info != (const void *)0 && device <= 1U &&
           handle->version == ATLAS_ATA_DEVICE_HANDLE_VERSION &&
           handle->command_base == command_base && handle->control_base == control_base &&
           handle->device == device && handle->logical_sector_count == info->logical_sector_count &&
           handle->identity_fingerprint == identity_fingerprint && identity_fingerprint != 0U;
}

int atlas_ata_identify_parse(
    const uint16_t words[ATLAS_ATA_IDENTIFY_WORDS], ATLAS_ATA_IDENTIFY_INFO *info) {
    uint64_t lba28;
    uint64_t lba48;

    if (words == (const void *)0 || info == (void *)0) {
        return 0;
    }
    lba28 = (uint64_t)words[60] | ((uint64_t)words[61] << 16U);
    lba48 = (uint64_t)words[100] | ((uint64_t)words[101] << 16U) |
            ((uint64_t)words[102] << 32U) | ((uint64_t)words[103] << 48U);
    if ((words[83] & ATLAS_ATA_LBA48_SUPPORTED) != 0U && lba48 != 0U) {
        info->logical_sector_count = lba48;
        info->lba48_supported = 1U;
        return 1;
    }
    if (lba28 == 0U) {
        return 0;
    }
    info->logical_sector_count = lba28;
    info->lba48_supported = 0U;
    return 1;
}

int atlas_ata_pio_identify(
    const ATLAS_ATA_PIO_TRANSPORT *transport,
    uint16_t command_base,
    uint16_t control_base,
    uint8_t device,
    ATLAS_ATA_IDENTIFY_INFO *info) {
    uint16_t words[ATLAS_ATA_IDENTIFY_WORDS];
    uint8_t status;
    uint32_t index;

    if (transport == (const void *)0 || transport->in8 == (void *)0 || transport->out8 == (void *)0 ||
        transport->in16 == (void *)0 || info == (void *)0 || device > 1U ||
        command_base > UINT16_MAX - ATLAS_ATA_REGISTER_STATUS_COMMAND) {
        return 0;
    }
    if (transport->in8(transport->context, command_base + ATLAS_ATA_REGISTER_STATUS_COMMAND) == UINT8_MAX) {
        return 0;
    }
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_DEVICE,
                    (uint8_t)(ATLAS_ATA_DEVICE_SELECT_BASE |
                              (device == 0U ? 0U : ATLAS_ATA_DEVICE_SELECT_SLAVE)));
    for (index = 0U; index < 15U; ++index) {
        (void)transport->in8(transport->context, control_base);
    }
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_SECTOR_COUNT, 0U);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_LOW, 0U);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_MID, 0U);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_HIGH, 0U);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_STATUS_COMMAND,
                    ATLAS_ATA_COMMAND_IDENTIFY_DEVICE);
    for (index = 0U; index < ATLAS_ATA_PIO_SPIN_LIMIT; ++index) {
        status = transport->in8(transport->context, control_base);
        if (status == 0U || (status & ATLAS_ATA_STATUS_BSY) == 0U) {
            break;
        }
    }
    if (index == ATLAS_ATA_PIO_SPIN_LIMIT || status == 0U ||
        transport->in8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_MID) != 0U ||
        transport->in8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_HIGH) != 0U) {
        return 0;
    }
    for (index = 0U; index < ATLAS_ATA_PIO_SPIN_LIMIT; ++index) {
        status = transport->in8(transport->context, control_base);
        if (status == 0U || (status & (ATLAS_ATA_STATUS_ERR | ATLAS_ATA_STATUS_DF)) != 0U) {
            return 0;
        }
        if ((status & ATLAS_ATA_STATUS_DRQ) != 0U) {
            uint32_t word_index;

            for (word_index = 0U; word_index < ATLAS_ATA_IDENTIFY_WORDS; ++word_index) {
                words[word_index] = transport->in16(transport->context, command_base + ATLAS_ATA_REGISTER_DATA);
            }
            return atlas_ata_identify_parse(words, info);
        }
    }
    return 0;
}

int atlas_ata_block_read_one(
    const ATLAS_ATA_PIO_TRANSPORT *transport,
    const ATLAS_ATA_DEVICE_HANDLE *handle,
    const ATLAS_ATA_IDENTIFY_INFO *info,
    uint64_t identity_fingerprint,
    uint32_t lba,
    uint16_t sector_words[ATLAS_ATA_SECTOR_WORDS]) {
    if (!atlas_ata_device_handle_matches(handle, handle == (const void *)0 ? 0U : handle->command_base,
                                         handle == (const void *)0 ? 0U : handle->control_base,
                                         handle == (const void *)0 ? 2U : handle->device, info,
                                         identity_fingerprint)) {
        return 0;
    }
    return atlas_ata_pio_read_sector_lba28(transport, handle->command_base, handle->control_base,
                                           handle->device, info, lba, sector_words);
}

int atlas_ata_pio_read_sector_lba28(
    const ATLAS_ATA_PIO_TRANSPORT *transport,
    uint16_t command_base,
    uint16_t control_base,
    uint8_t device,
    const ATLAS_ATA_IDENTIFY_INFO *info,
    uint32_t lba,
    uint16_t sector_words[ATLAS_ATA_SECTOR_WORDS]) {
    uint8_t status;
    uint32_t index;

    if (transport == (const void *)0 || transport->in8 == (void *)0 || transport->out8 == (void *)0 ||
        transport->in16 == (void *)0 || info == (const void *)0 || sector_words == (void *)0 ||
        device > 1U || info->logical_sector_count == 0U || lba >= ATLAS_ATA_LBA28_SECTOR_LIMIT ||
        (uint64_t)lba >= info->logical_sector_count ||
        command_base > UINT16_MAX - ATLAS_ATA_REGISTER_STATUS_COMMAND) {
        return 0;
    }
    if (transport->in8(transport->context, command_base + ATLAS_ATA_REGISTER_STATUS_COMMAND) == UINT8_MAX) {
        return 0;
    }
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_DEVICE,
                    (uint8_t)(ATLAS_ATA_DEVICE_SELECT_BASE | ATLAS_ATA_DEVICE_LBA_MODE |
                              (device == 0U ? 0U : ATLAS_ATA_DEVICE_SELECT_SLAVE) |
                              (uint8_t)((lba >> 24U) & UINT32_C(0x0f))));
    for (index = 0U; index < 15U; ++index) {
        (void)transport->in8(transport->context, control_base);
    }
    status = transport->in8(transport->context, control_base);
    if ((status & (ATLAS_ATA_STATUS_BSY | ATLAS_ATA_STATUS_DRQ)) != 0U ||
        (status & (ATLAS_ATA_STATUS_ERR | ATLAS_ATA_STATUS_DF)) != 0U) {
        return 0;
    }
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_SECTOR_COUNT, 1U);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_LOW, (uint8_t)lba);
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_MID, (uint8_t)(lba >> 8U));
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_LBA_HIGH, (uint8_t)(lba >> 16U));
    transport->out8(transport->context, command_base + ATLAS_ATA_REGISTER_STATUS_COMMAND,
                    ATLAS_ATA_COMMAND_READ_SECTORS);
    for (index = 0U; index < ATLAS_ATA_PIO_SPIN_LIMIT; ++index) {
        status = transport->in8(transport->context, control_base);
        if (status == 0U || (status & (ATLAS_ATA_STATUS_ERR | ATLAS_ATA_STATUS_DF)) != 0U) {
            return 0;
        }
        if ((status & ATLAS_ATA_STATUS_BSY) == 0U && (status & ATLAS_ATA_STATUS_DRQ) != 0U) {
            uint32_t word_index;

            for (word_index = 0U; word_index < ATLAS_ATA_SECTOR_WORDS; ++word_index) {
                sector_words[word_index] = transport->in16(transport->context, command_base + ATLAS_ATA_REGISTER_DATA);
            }
            return 1;
        }
    }
    return 0;
}

int atlas_ata_runtime_probe(void) {
    const ATLAS_ATA_PIO_TRANSPORT transport = {
        atlas_ata_x86_in8, atlas_ata_x86_out8, atlas_ata_x86_in16, (void *)0};
    ATLAS_ATA_IDENTIFY_INFO info;

    return atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                                  ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info) &&
           info.logical_sector_count != 0U;
}

int atlas_ata_runtime_sector_read_probe(void) {
    const ATLAS_ATA_PIO_TRANSPORT transport = {
        atlas_ata_x86_in8, atlas_ata_x86_out8, atlas_ata_x86_in16, (void *)0};
    ATLAS_ATA_IDENTIFY_INFO info;
    uint16_t sector_words[ATLAS_ATA_SECTOR_WORDS];

    return atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                                  ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info) &&
           atlas_ata_pio_read_sector_lba28(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                                           ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info, 0U,
                                           sector_words);
}
