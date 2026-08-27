/*
 * VibeOS Prelude — native x86_64 UEFI loader entry point.
 *
 * Prelude owns the final UEFI transition, seals Dawn Context, places the
 * independent Pulse image at its fixed early physical address, and transfers
 * control with the x86_64 System V ABI. Pulse never receives live Boot Services.
 */

#include "uefi.h"
#include <dawn.h>

#define PRELUDE_PULSE_LOAD_ADDRESS UINT64_C(0x00200000)
#define PRELUDE_PAGE_SIZE ((UINTN)4096)
#define PRELUDE_PULSE_STACK_PAGES ((UINTN)32)

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

static EFI_GUID prelude_graphics_output_protocol_guid = {
    UINT32_C(0x9042a9de), 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

static void prelude_copy_bytes(void *destination, const void *source, UINTN size) {
    uint8_t *output = destination;
    const uint8_t *input = source;
    UINTN index;

    for (index = 0; index < size; ++index) {
        output[index] = input[index];
    }
}

static EFI_STATUS prelude_capture_dawn_context(EFI_SYSTEM_TABLE *system_table) {
    EFI_BOOT_SERVICES *boot_services = system_table->boot_services;
    EFI_STATUS status;
    UINTN memory_map_size = 0;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    UINT32 descriptor_version = 0;
    UINTN allocation_size;
    EFI_MEMORY_DESCRIPTOR *memory_map = (void *)0;

    status = boot_services->get_memory_map(
        &memory_map_size, (void *)0, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_BUFFER_TOO_SMALL || descriptor_size == 0) {
        return status;
    }

    allocation_size = memory_map_size + (descriptor_size * 8U);
    status = boot_services->allocate_pool(EFI_LOADER_DATA, allocation_size, (void **)&memory_map);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = boot_services->get_memory_map(
        &allocation_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_SUCCESS) {
        return status;
    }

    dawn_context.magic = DAWN_CONTEXT_MAGIC;
    dawn_context.version = DAWN_CONTEXT_VERSION;
    dawn_context.size = (uint32_t)sizeof(dawn_context);
    dawn_context.memory_map_physical_address = (uint64_t)(UINTN)memory_map;
    dawn_context.memory_map_size = (uint64_t)allocation_size;
    dawn_context.memory_map_key = (uint64_t)map_key;
    dawn_context.memory_descriptor_size = (uint64_t)descriptor_size;
    dawn_context.memory_descriptor_version = descriptor_version;
    dawn_context.reserved = 0;
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
