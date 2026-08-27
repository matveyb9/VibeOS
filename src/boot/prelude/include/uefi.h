/*
 * VibeOS Prelude — focused UEFI definitions for the x86_64 bootstrap path.
 *
 * The declarations intentionally cover only services Prelude invokes. New UEFI
 * protocols enter this boundary only with a defined Prelude responsibility.
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
typedef UINT64 EFI_PHYSICAL_ADDRESS;

#define EFI_SUCCESS ((EFI_STATUS)0)
#define EFI_LOAD_ERROR ((EFI_STATUS)0x8000000000000001ULL)
#define EFI_INVALID_PARAMETER ((EFI_STATUS)0x8000000000000002ULL)
#define EFI_UNSUPPORTED ((EFI_STATUS)0x8000000000000003ULL)
#define EFI_BUFFER_TOO_SMALL ((EFI_STATUS)0x8000000000000005ULL)
#define EFI_OUT_OF_RESOURCES ((EFI_STATUS)0x8000000000000009ULL)
#define EFI_ALLOCATE_ANY_PAGES ((UINT32)0)
#define EFI_ALLOCATE_ADDRESS ((UINT32)2)
#define EFI_LOADER_DATA ((UINT32)2)

typedef struct {
    UINT64 signature;
    UINT32 revision;
    UINT32 header_size;
    UINT32 crc32;
    UINT32 reserved;
} EFI_TABLE_HEADER;

typedef struct {
    UINT32 data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
} EFI_GUID;

typedef struct {
    UINT32 type;
    UINT32 padding;
    UINT64 physical_start;
    UINT64 virtual_start;
    UINT64 number_of_pages;
    UINT64 attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
    EFI_PIXEL_RGB_RESERVED_8_BIT_PER_COLOR = 0,
    EFI_PIXEL_BGR_RESERVED_8_BIT_PER_COLOR = 1,
    EFI_PIXEL_BIT_MASK = 2,
    EFI_PIXEL_BLT_ONLY = 3
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32 red_mask;
    UINT32 green_mask;
    UINT32 blue_mask;
    UINT32 reserved_mask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT32 version;
    UINT32 horizontal_resolution;
    UINT32 vertical_resolution;
    EFI_GRAPHICS_PIXEL_FORMAT pixel_format;
    EFI_PIXEL_BITMASK pixel_information;
    UINT32 pixels_per_scan_line;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 max_mode;
    UINT32 mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN size_of_info;
    EFI_PHYSICAL_ADDRESS frame_buffer_base;
    UINTN frame_buffer_size;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    void *query_mode;
    void *set_mode;
    void *blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_PAGES)(
    UINT32 type, UINT32 memory_type, UINTN pages, EFI_PHYSICAL_ADDRESS *memory);
typedef EFI_STATUS(EFIAPI *EFI_GET_MEMORY_MAP)(
    UINTN *memory_map_size,
    EFI_MEMORY_DESCRIPTOR *memory_map,
    UINTN *map_key,
    UINTN *descriptor_size,
    UINT32 *descriptor_version);
typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_POOL)(UINT32 pool_type, UINTN size, void **buffer);
typedef EFI_STATUS(EFIAPI *EFI_HANDLE_PROTOCOL)(EFI_HANDLE handle, EFI_GUID *protocol, void **interface);
typedef EFI_STATUS(EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE image_handle, UINTN map_key);

typedef struct {
    EFI_TABLE_HEADER header;
    void *raise_tpl;
    void *restore_tpl;
    EFI_ALLOCATE_PAGES allocate_pages;
    void *free_pages;
    EFI_GET_MEMORY_MAP get_memory_map;
    EFI_ALLOCATE_POOL allocate_pool;
    void *free_pool;
    void *create_event;
    void *set_timer;
    void *wait_for_event;
    void *signal_event;
    void *close_event;
    void *check_event;
    void *install_protocol_interface;
    void *reinstall_protocol_interface;
    void *uninstall_protocol_interface;
    EFI_HANDLE_PROTOCOL handle_protocol;
    void *reserved;
    void *register_protocol_notify;
    void *locate_handle;
    void *locate_device_path;
    void *install_configuration_table;
    void *load_image;
    void *start_image;
    void *exit;
    void *unload_image;
    EFI_EXIT_BOOT_SERVICES exit_boot_services;
} EFI_BOOT_SERVICES;

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
    UINT32 revision;
    EFI_HANDLE parent_handle;
    struct EFI_SYSTEM_TABLE *system_table;
    EFI_HANDLE device_handle;
    void *file_path;
    void *reserved;
    UINT32 load_options_size;
    void *load_options;
    void *image_base;
    UINTN image_size;
    UINT32 image_code_type;
    UINT32 image_data_type;
    void *unload;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef struct EFI_SYSTEM_TABLE {
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
    EFI_BOOT_SERVICES *boot_services;
    UINTN number_of_table_entries;
    void *configuration_table;
} EFI_SYSTEM_TABLE;

#endif
