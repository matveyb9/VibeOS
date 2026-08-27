/*
 * VibeOS Prelude — minimal UEFI definitions for the first x86_64 bring-up.
 *
 * This header intentionally declares only the UEFI structures Prelude uses.
 * It is not a copied general-purpose UEFI SDK and will grow through explicit
 * interfaces as Prelude gains responsibilities.
 */

#ifndef VIBEOS_PRELUDE_UEFI_H
#define VIBEOS_PRELUDE_UEFI_H

#include <stdint.h>

#if defined(__x86_64__)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif

typedef void *EFI_HANDLE;
typedef uint64_t EFI_STATUS;
typedef uint64_t UINTN;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint16_t CHAR16;

#define EFI_SUCCESS ((EFI_STATUS)0)

typedef struct {
    UINT64 signature;
    UINT32 revision;
    UINT32 header_size;
    UINT32 crc32;
    UINT32 reserved;
} EFI_TABLE_HEADER;

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_TEXT_RESET)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self,
    uint8_t extended_verification);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_STRING)(
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self,
    CHAR16 *string);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET reset;
    EFI_TEXT_STRING output_string;
    void *test_string;
    void *query_mode;
    void *set_mode;
    void *set_attribute;
    void *clear_screen;
    void *set_cursor_position;
    void *enable_cursor;
    void *mode;
};

typedef struct {
    EFI_TABLE_HEADER header;
    CHAR16 *firmware_vendor;
    UINT32 firmware_revision;
    EFI_HANDLE console_in_handle;
    void *con_in;
    EFI_HANDLE console_out_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *con_out;
    EFI_HANDLE standard_error_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *std_err;
    void *runtime_services;
    void *boot_services;
    UINTN number_of_table_entries;
    void *configuration_table;
} EFI_SYSTEM_TABLE;

#endif
