/* Dawn ACPI RSDP test — checks bounded signature, version, and checksum rules. */

#include <stdint.h>
#include <stdio.h>

#include "dawn_acpi.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

static void apply_checksum(uint8_t *bytes, uint32_t length, uint32_t checksum_offset) {
    uint8_t checksum = 0U;
    uint32_t index;

    bytes[checksum_offset] = 0U;
    for (index = 0U; index < length; ++index) {
        checksum = (uint8_t)(checksum + bytes[index]);
    }
    bytes[checksum_offset] = (uint8_t)(0U - checksum);
}

static void make_valid_rsdp_v2(uint8_t rsdp[36]) {
    uint32_t index;
    static const uint8_t signature[8] = {'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};

    for (index = 0U; index < 36U; ++index) {
        rsdp[index] = 0U;
    }
    for (index = 0U; index < sizeof(signature); ++index) {
        rsdp[index] = signature[index];
    }
    rsdp[15] = 2U;
    rsdp[20] = 36U;
    apply_checksum(rsdp, 20U, 8U);
    apply_checksum(rsdp, 36U, 32U);
}

int main(void) {
    uint8_t rsdp[36];

    make_valid_rsdp_v2(rsdp);
    if (!expect(!dawn_acpi_rsdp_is_valid((const void *)0), "null pointer is rejected") ||
        !expect(dawn_acpi_rsdp_is_valid(rsdp), "valid ACPI 2.0 RSDP is accepted")) {
        return 1;
    }
    rsdp[0] = 'X';
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "wrong RSDP signature is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[20] = 35U;
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "short ACPI 2.0 RSDP length is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[8] = (uint8_t)(rsdp[8] + 1U);
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "bad legacy checksum is rejected")) {
        return 1;
    }
    make_valid_rsdp_v2(rsdp);
    rsdp[32] = (uint8_t)(rsdp[32] + 1U);
    if (!expect(!dawn_acpi_rsdp_is_valid(rsdp), "bad extended checksum is rejected")) {
        return 1;
    }

    puts("Dawn ACPI RSDP unit tests passed.");
    return 0;
}
