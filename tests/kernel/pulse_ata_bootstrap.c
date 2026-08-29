#include <atlas_ata.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t words[ATLAS_ATA_IDENTIFY_WORDS];
    uint32_t data_index;
    uint32_t output_count;
    uint8_t command_issued;
    uint8_t floating;
    uint8_t error;
    uint8_t last_command;
} FAKE_ATA;

static uint8_t fake_in8(void *context, uint16_t port) {
    FAKE_ATA *fake = (FAKE_ATA *)context;

    if (port == ATLAS_ATA_PRIMARY_COMMAND_BASE + 7U && fake->floating != 0U) {
        return UINT8_MAX;
    }
    if (port == ATLAS_ATA_PRIMARY_COMMAND_BASE + 4U || port == ATLAS_ATA_PRIMARY_COMMAND_BASE + 5U) {
        return 0U;
    }
    if (port == ATLAS_ATA_PRIMARY_CONTROL_BASE && fake->command_issued != 0U) {
        if (fake->error != 0U) {
            return UINT8_C(0x01);
        }
        if (fake->data_index == UINT32_MAX) {
            fake->data_index = 0U;
            return UINT8_C(0x80);
        }
        return UINT8_C(0x08);
    }
    return UINT8_C(0x40);
}

static void fake_out8(void *context, uint16_t port, uint8_t value) {
    FAKE_ATA *fake = (FAKE_ATA *)context;

    ++fake->output_count;
    if (port == ATLAS_ATA_PRIMARY_COMMAND_BASE + 7U) {
        fake->command_issued = 1U;
        fake->last_command = value;
    }
}

static uint16_t fake_in16(void *context, uint16_t port) {
    FAKE_ATA *fake = (FAKE_ATA *)context;
    uint16_t value;

    if (port != ATLAS_ATA_PRIMARY_COMMAND_BASE || fake->data_index >= ATLAS_ATA_IDENTIFY_WORDS) {
        return 0U;
    }
    value = fake->words[fake->data_index];
    ++fake->data_index;
    return value;
}

static void fake_initialize(FAKE_ATA *fake) {
    uint32_t index;

    for (index = 0U; index < ATLAS_ATA_IDENTIFY_WORDS; ++index) {
        fake->words[index] = 0U;
    }
    fake->data_index = UINT32_MAX;
    fake->output_count = 0U;
    fake->command_issued = 0U;
    fake->floating = 0U;
    fake->error = 0U;
    fake->last_command = 0U;
    fake->words[60] = UINT16_C(0x1234);
    fake->words[61] = UINT16_C(0x0002);
}

int main(void) {
    FAKE_ATA fake;
    ATLAS_ATA_IDENTIFY_INFO info;
    ATLAS_ATA_PIO_TRANSPORT transport = {fake_in8, fake_out8, fake_in16, &fake};

    fake_initialize(&fake);
    if (!atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                                ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info) ||
        info.logical_sector_count != UINT64_C(0x21234) || info.lba48_supported != 0U ||
        fake.last_command != UINT8_C(0xec) || fake.output_count != 6U) {
        fputs("Atlas ATA PIO identify test failed.\n", stderr);
        return 1;
    }
    fake_initialize(&fake);
    fake.words[83] = UINT16_C(0x0400);
    fake.words[100] = UINT16_C(0x0001);
    fake.words[102] = UINT16_C(0x0001);
    if (!atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                                ATLAS_ATA_PRIMARY_CONTROL_BASE, 1U, &info) ||
        info.logical_sector_count != UINT64_C(0x100000001) || info.lba48_supported != 1U) {
        fputs("Atlas ATA LBA48 identify test failed.\n", stderr);
        return 1;
    }
    fake_initialize(&fake);
    fake.floating = 1U;
    if (atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                               ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info)) {
        fputs("Atlas ATA floating bus was accepted.\n", stderr);
        return 1;
    }
    fake_initialize(&fake);
    fake.error = 1U;
    if (atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                               ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info) ||
        atlas_ata_pio_identify(&transport, UINT16_MAX, ATLAS_ATA_PRIMARY_CONTROL_BASE, 0U, &info) ||
        atlas_ata_pio_identify(&transport, ATLAS_ATA_PRIMARY_COMMAND_BASE,
                               ATLAS_ATA_PRIMARY_CONTROL_BASE, 2U, &info)) {
        fputs("Atlas ATA invalid input was accepted.\n", stderr);
        return 1;
    }
    puts("Atlas ATA PIO identify unit tests passed.");
    return 0;
}
