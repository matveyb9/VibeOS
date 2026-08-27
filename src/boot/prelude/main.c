/*
 * VibeOS Prelude — native x86_64 UEFI loader entry point.
 *
 * Prelude owns the final UEFI transition, seals Dawn Context, places the
 * independent Pulse image at its fixed early physical address, and transfers
 * control with the x86_64 System V ABI. Pulse never receives live Boot Services.
 */

#include "uefi.h"
#include <dawn.h>
#include <dawn_acpi.h>

#define PRELUDE_PULSE_LOAD_ADDRESS UINT64_C(0x00200000)
#define PRELUDE_PAGE_SIZE ((UINTN)4096)
#define PRELUDE_PULSE_STACK_PAGES ((UINTN)32)
#define PRELUDE_BOOT_RESERVATION_CAPACITY ((uint32_t)2)

typedef void(__attribute__((sysv_abi)) *PRELUDE_PULSE_ENTRY)(const DAWN_CONTEXT *context);

extern const uint8_t prelude_pulse_image_begin[];
extern const uint8_t prelude_pulse_image_end[];

static CHAR16 prelude_banner[] = {
    'V', 'i', 'b', 'e', 'O', 'S', ' ', 'P', 'r', 'e', 'l', 'u', 'd', 'e',
    ' ', '0', '.', '0', '.', '2', '\r', '\n',
    'S', 'e', 'a', 'l', 'i', 'n', 'g', ' ', 'D', 'a', 'w', 'n', ' ', 'C', 'o', 'n', 't', 'e', 'x', 't',
    '.', '\r', '\n', 0
};

static DAWN_CONTEXT dawn_context;
static DAWN_MEMORY_RANGE prelude_boot_reservations[PRELUDE_BOOT_RESERVATION_CAPACITY];
static uint32_t prelude_boot_reservation_count;

static EFI_GUID prelude_graphics_output_protocol_guid = {
    UINT32_C(0x9042a9de), 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
static EFI_GUID prelude_acpi_20_table_guid = {
    UINT32_C(0x8868e871), 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
static EFI_GUID prelude_acpi_10_table_guid = {
    UINT32_C(0xeb9d2d30), 0x2d88, 0x11d3, {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};

static void prelude_copy_bytes(void *destination, const void *source, UINTN size) {
    uint8_t *output = destination;
    const uint8_t *input = source;
    UINTN index;

    for (index = 0; index < size; ++index) {
        output[index] = input[index];
    }
}

static int prelude_guid_equal(const EFI_GUID *left, const EFI_GUID *right) {
    UINTN index;

    if (left->data1 != right->data1 || left->data2 != right->data2 || left->data3 != right->data3) {
        return 0;
    }
    for (index = 0U; index < sizeof(left->data4); ++index) {
        if (left->data4[index] != right->data4[index]) {
            return 0;
        }
    }
    return 1;
}

static EFI_STATUS prelude_capture_acpi_rsdp(EFI_SYSTEM_TABLE *system_table) {
    EFI_CONFIGURATION_TABLE *fallback = (void *)0;
    UINTN table_index;

    if (system_table->configuration_table == (void *)0) {
        return EFI_UNSUPPORTED;
    }
    for (table_index = 0U; table_index < system_table->number_of_table_entries; ++table_index) {
        EFI_CONFIGURATION_TABLE *table = &system_table->configuration_table[table_index];

        if (prelude_guid_equal(&table->vendor_guid, &prelude_acpi_20_table_guid)) {
            if (table->vendor_table == (void *)0 ||
                (uint64_t)(UINTN)table->vendor_table > UINT64_C(0xffffffff) ||
                !dawn_acpi_rsdp_is_valid(table->vendor_table)) {
                return EFI_LOAD_ERROR;
            }
            dawn_context.acpi_rsdp_physical_address = (uint64_t)(UINTN)table->vendor_table;
            return EFI_SUCCESS;
        }
        if (prelude_guid_equal(&table->vendor_guid, &prelude_acpi_10_table_guid)) {
            fallback = table;
        }
    }
    if (fallback == (void *)0 || fallback->vendor_table == (void *)0 ||
        (uint64_t)(UINTN)fallback->vendor_table > UINT64_C(0xffffffff) ||
        !dawn_acpi_rsdp_is_valid(fallback->vendor_table)) {
        return EFI_UNSUPPORTED;
    }
    dawn_context.acpi_rsdp_physical_address = (uint64_t)(UINTN)fallback->vendor_table;
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_record_boot_reservation(uint64_t physical_start, uint64_t byte_size) {
    uint32_t index;
    uint32_t insert_index;

    if (byte_size == 0U || physical_start > UINT64_MAX - byte_size ||
        prelude_boot_reservation_count >= PRELUDE_BOOT_RESERVATION_CAPACITY) {
        return EFI_OUT_OF_RESOURCES;
    }

    insert_index = prelude_boot_reservation_count;
    while (insert_index > 0U &&
           physical_start < prelude_boot_reservations[insert_index - 1U].physical_start) {
        prelude_boot_reservations[insert_index] = prelude_boot_reservations[insert_index - 1U];
        --insert_index;
    }
    prelude_boot_reservations[insert_index].physical_start = physical_start;
    prelude_boot_reservations[insert_index].byte_size = byte_size;
    ++prelude_boot_reservation_count;

    for (index = 1U; index < prelude_boot_reservation_count; ++index) {
        uint64_t previous_limit = prelude_boot_reservations[index - 1U].physical_start +
                                  prelude_boot_reservations[index - 1U].byte_size;

        if (previous_limit > prelude_boot_reservations[index].physical_start) {
            return EFI_LOAD_ERROR;
        }
    }
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_capture_dawn_context(EFI_SYSTEM_TABLE *system_table) {
    EFI_BOOT_SERVICES *boot_services = system_table->boot_services;
    EFI_STATUS status;
    UINTN memory_map_size = 0;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;
    UINTN memory_map_capacity;
    UINTN descriptor_count;
    UINTN descriptor_index;
    EFI_MEMORY_DESCRIPTOR *memory_map = (void *)0;
    DAWN_MEMORY_DESCRIPTOR *dawn_memory_map = (void *)0;

    status = prelude_capture_acpi_rsdp(system_table);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = boot_services->get_memory_map(
        &memory_map_size, (void *)0, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_BUFFER_TOO_SMALL || descriptor_size == 0) {
        return status;
    }

    memory_map_capacity = memory_map_size + (descriptor_size * 8U);
    status = boot_services->allocate_pool(EFI_LOADER_DATA, memory_map_capacity, (void **)&memory_map);
    if (status != EFI_SUCCESS) {
        return status;
    }

    memory_map_size = memory_map_capacity;
    status = boot_services->get_memory_map(
        &memory_map_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_SUCCESS) {
        return status;
    }

    if (descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR) || memory_map_size == 0U ||
        (memory_map_size % descriptor_size) != 0U) {
        return EFI_LOAD_ERROR;
    }
    descriptor_count = memory_map_capacity / descriptor_size;
    status = boot_services->allocate_pool(
        EFI_LOADER_DATA, descriptor_count * sizeof(DAWN_MEMORY_DESCRIPTOR), (void **)&dawn_memory_map);
    if (status != EFI_SUCCESS) {
        return status;
    }

    memory_map_size = memory_map_capacity;
    status = boot_services->get_memory_map(
        &memory_map_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_SUCCESS || descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR) || memory_map_size == 0U ||
        (memory_map_size % descriptor_size) != 0U) {
        return status == EFI_SUCCESS ? EFI_LOAD_ERROR : status;
    }
    descriptor_count = memory_map_size / descriptor_size;
    for (descriptor_index = 0; descriptor_index < descriptor_count; ++descriptor_index) {
        const EFI_MEMORY_DESCRIPTOR *descriptor =
            (const EFI_MEMORY_DESCRIPTOR *)((const uint8_t *)memory_map + (descriptor_index * descriptor_size));

        dawn_memory_map[descriptor_index].physical_start = descriptor->physical_start;
        dawn_memory_map[descriptor_index].byte_size = descriptor->number_of_pages * PRELUDE_PAGE_SIZE;
        dawn_memory_map[descriptor_index].kind =
            descriptor->type == UINT32_C(7) ? DAWN_MEMORY_USABLE : DAWN_MEMORY_RESERVED;
        dawn_memory_map[descriptor_index].attributes = 0U;
    }

    dawn_context.magic = DAWN_CONTEXT_MAGIC;
    dawn_context.version = DAWN_CONTEXT_VERSION;
    dawn_context.size = (uint32_t)sizeof(dawn_context);
    dawn_context.memory_map_physical_address = (uint64_t)(UINTN)dawn_memory_map;
    dawn_context.memory_map_size = descriptor_count * sizeof(DAWN_MEMORY_DESCRIPTOR);
    dawn_context.memory_map_key = (uint64_t)map_key;
    dawn_context.memory_descriptor_size = sizeof(DAWN_MEMORY_DESCRIPTOR);
    dawn_context.memory_descriptor_version = DAWN_MEMORY_DESCRIPTOR_VERSION;
    dawn_context.reserved = 0;
    dawn_context.boot_reservations_physical_address = (uint64_t)(UINTN)prelude_boot_reservations;
    dawn_context.boot_reservations_size =
        (uint64_t)prelude_boot_reservation_count * sizeof(DAWN_MEMORY_RANGE);
    dawn_context.boot_reservation_descriptor_size = sizeof(DAWN_MEMORY_RANGE);
    dawn_context.boot_reservation_descriptor_version = DAWN_MEMORY_RANGE_VERSION;
    dawn_context.boot_reservation_count = prelude_boot_reservation_count;
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_capture_framebuffer(EFI_SYSTEM_TABLE *system_table) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_output = (void *)0;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *information;
    EFI_STATUS status;

    if (system_table->console_out_handle == (void *)0 ||
        system_table->boot_services->handle_protocol == (void *)0) {
        return EFI_UNSUPPORTED;
    }
    status = system_table->boot_services->handle_protocol(
        system_table->console_out_handle, &prelude_graphics_output_protocol_guid, (void **)&graphics_output);
    if (status != EFI_SUCCESS || graphics_output == (void *)0 || graphics_output->mode == (void *)0 ||
        graphics_output->mode->info == (void *)0) {
        return status != EFI_SUCCESS ? status : EFI_UNSUPPORTED;
    }

    mode = graphics_output->mode;
    information = mode->info;
    if (mode->frame_buffer_base == 0U || mode->frame_buffer_size == 0U ||
        information->horizontal_resolution == 0U || information->vertical_resolution == 0U ||
        information->pixels_per_scan_line < information->horizontal_resolution) {
        return EFI_UNSUPPORTED;
    }
    if (information->pixel_format == EFI_PIXEL_RGB_RESERVED_8_BIT_PER_COLOR) {
        dawn_context.framebuffer_pixel_format = DAWN_PIXEL_FORMAT_RGBX8888;
    } else if (information->pixel_format == EFI_PIXEL_BGR_RESERVED_8_BIT_PER_COLOR) {
        dawn_context.framebuffer_pixel_format = DAWN_PIXEL_FORMAT_BGRX8888;
    } else {
        return EFI_UNSUPPORTED;
    }
    dawn_context.framebuffer_physical_address = mode->frame_buffer_base;
    dawn_context.framebuffer_byte_size = mode->frame_buffer_size;
    dawn_context.framebuffer_width = information->horizontal_resolution;
    dawn_context.framebuffer_height = information->vertical_resolution;
    dawn_context.framebuffer_pixels_per_scan_line = information->pixels_per_scan_line;
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_load_pulse(
    EFI_SYSTEM_TABLE *system_table, EFI_PHYSICAL_ADDRESS *entry_address) {
    EFI_STATUS status;
    UINTN pulse_size = (UINTN)(prelude_pulse_image_end - prelude_pulse_image_begin);
    UINTN pages;
    EFI_PHYSICAL_ADDRESS load_address = PRELUDE_PULSE_LOAD_ADDRESS;

    if (pulse_size == 0 || entry_address == (void *)0) {
        return EFI_LOAD_ERROR;
    }

    pages = (pulse_size + (PRELUDE_PAGE_SIZE - 1U)) / PRELUDE_PAGE_SIZE;
    status = system_table->boot_services->allocate_pages(
        EFI_ALLOCATE_ADDRESS, EFI_LOADER_DATA, pages, &load_address);
    if (status != EFI_SUCCESS || load_address != PRELUDE_PULSE_LOAD_ADDRESS) {
        return status == EFI_SUCCESS ? EFI_OUT_OF_RESOURCES : status;
    }

    prelude_copy_bytes((void *)(UINTN)load_address, prelude_pulse_image_begin, pulse_size);
    status = prelude_record_boot_reservation(load_address, pages * PRELUDE_PAGE_SIZE);
    if (status != EFI_SUCCESS) {
        return status;
    }
    *entry_address = load_address;
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_prepare_pulse_stack(EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status;
    EFI_PHYSICAL_ADDRESS stack_base = 0;

    status = system_table->boot_services->allocate_pages(
        EFI_ALLOCATE_ANY_PAGES, EFI_LOADER_DATA, PRELUDE_PULSE_STACK_PAGES, &stack_base);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = prelude_record_boot_reservation(
        stack_base, PRELUDE_PULSE_STACK_PAGES * PRELUDE_PAGE_SIZE);
    if (status != EFI_SUCCESS) {
        return status;
    }
    dawn_context.kernel_stack_top = stack_base + (PRELUDE_PULSE_STACK_PAGES * PRELUDE_PAGE_SIZE);
    dawn_context.kernel_stack_size = PRELUDE_PULSE_STACK_PAGES * PRELUDE_PAGE_SIZE;
    return EFI_SUCCESS;
}

static EFI_STATUS prelude_seal_dawn_context(
    EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status = EFI_SUCCESS;
    UINTN attempt;

    for (attempt = 0; attempt < 3U; ++attempt) {
        status = prelude_capture_dawn_context(system_table);
        if (status != EFI_SUCCESS) {
            return status;
        }

        status = system_table->boot_services->exit_boot_services(image_handle, (UINTN)dawn_context.memory_map_key);
        if (status == EFI_SUCCESS) {
            return EFI_SUCCESS;
        }
    }

    return status;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status;
    EFI_PHYSICAL_ADDRESS pulse_entry_address = 0;
    PRELUDE_PULSE_ENTRY pulse_entry;

    if (system_table == (void *)0 || system_table->boot_services == (void *)0 ||
        system_table->boot_services->allocate_pages == (void *)0 ||
        system_table->boot_services->get_memory_map == (void *)0 ||
        system_table->boot_services->allocate_pool == (void *)0 ||
        system_table->boot_services->handle_protocol == (void *)0 ||
        system_table->boot_services->exit_boot_services == (void *)0) {
        return EFI_INVALID_PARAMETER;
    }

    if (system_table->con_out != (void *)0 && system_table->con_out->output_string != (void *)0) {
        system_table->con_out->output_string(system_table->con_out, prelude_banner);
    }

    status = prelude_load_pulse(system_table, &pulse_entry_address);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = prelude_prepare_pulse_stack(system_table);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = prelude_capture_framebuffer(system_table);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = prelude_seal_dawn_context(image_handle, system_table);
    if (status != EFI_SUCCESS) {
        return status;
    }

    pulse_entry = (PRELUDE_PULSE_ENTRY)(UINTN)pulse_entry_address;
    pulse_entry(&dawn_context);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
