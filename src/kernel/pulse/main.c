/*
 * VibeOS Pulse — first freestanding x86_64 kernel entry.
 *
 * Pulse validates Dawn Context before it trusts firmware-supplied memory data.
 * Paging, physical allocation, interrupts, and tasking are deliberately later
 * responsibilities; this step proves the native Prelude-to-Pulse boundary.
 */

#include "memory.h"
#include "paging.h"
#include "interrupts.h"
#include "interrupt-controller.h"
#include "scheduler.h"
#include "context.h"
#include "timer.h"
#include <dawn_acpi.h>
#include <origin.h>
#include <vaultfs.h>
#include <session_mode.h>
#include <parcel.h>
#include <prism.h>
#include <horizon.h>
#include <horizon_input.h>
#include <horizon_runtime.h>
#include <atlas_input.h>
#include <atlas_pci.h>

int prism_canvas_runtime_probe(const DAWN_CONTEXT *context);

static void pulse_debug_putc(char character) {
    __asm__ volatile("outb %0, %w1" : : "a"(character), "d"((uint16_t)0x402));
}

static void pulse_debug_write(const char *message) {
    while (*message != '\0') {
        pulse_debug_putc(*message);
        ++message;
    }
}

static void pulse_debug_exit(void) {
    __asm__ volatile("outl %0, %w1" : : "a"(0), "d"((uint16_t)0x0f4));
}

static int pulse_acpi_read_lower_4g(
    uint64_t physical_address, uint32_t byte_count, uint8_t *destination, void *reader_context) {
    const uint8_t *source;
    uint32_t index;

    (void)reader_context;
    if (destination == (void *)0 || physical_address > UINT32_MAX ||
        physical_address + byte_count < physical_address || physical_address + byte_count > UINT64_C(0x100000000)) {
        return 0;
    }
    source = (const uint8_t *)(uintptr_t)physical_address;
    for (index = 0U; index < byte_count; ++index) {
        destination[index] = source[index];
    }
    return 1;
}

static int pulse_acpi_child_table_inventory_probe(const DAWN_CONTEXT *context) {
    DAWN_ACPI_ROOT_TABLE_METADATA root_table;
    DAWN_ACPI_CHILD_TABLE_INVENTORY inventory;
    DAWN_ACPI_MADT_INVENTORY madt;
    DAWN_ACPI_MADT_X86_INVENTORY madt_x86;
    PULSE_INTERRUPT_CONTROLLER_SELECTION controller_selection;
    PULSE_X86_APIC_HANDOFF_PLAN apic_handoff_plan;
    PULSE_TIMER_SOURCE_SELECTION timer_selection;
    PULSE_TIMER_CAPABILITIES timer_capabilities;
    uint32_t index;

    if (context == (void *)0 ||
        !dawn_acpi_root_table_describe((const void *)(uintptr_t)context->acpi_rsdp_physical_address, &root_table) ||
        !dawn_acpi_child_table_inventory(&root_table, pulse_acpi_read_lower_4g, (void *)0, &inventory)) {
        return 0;
    }
    for (index = 0U; index < inventory.entry_count; ++index) {
        if (inventory.entries[index].signature[0] == 'A' && inventory.entries[index].signature[1] == 'P' &&
            inventory.entries[index].signature[2] == 'I' && inventory.entries[index].signature[3] == 'C' &&
            (inventory.entries[index].status & DAWN_ACPI_CHILD_TABLE_CHECKSUM_VALID) != 0U) {
            return dawn_acpi_madt_inventory(
                       inventory.entries[index].physical_address, pulse_acpi_read_lower_4g, (void *)0, &madt) &&
                   dawn_acpi_madt_x86_inventory(&madt, pulse_acpi_read_lower_4g, (void *)0, &madt_x86) &&
                   pulse_interrupt_controller_select(&madt_x86, &controller_selection) &&
                   controller_selection.active_controller == PULSE_INTERRUPT_CONTROLLER_PIC &&
                   pulse_x86_apic_handoff_plan_build(&madt, &madt_x86, &apic_handoff_plan) &&
                   pulse_x86_apic_handoff_plan_is_ready(&apic_handoff_plan) &&
                   pulse_timer_source_select(&controller_selection, &apic_handoff_plan, &timer_selection) &&
                   timer_selection.active_source == PULSE_TIMER_SOURCE_PIT &&
                   pulse_timer_capabilities_describe(&timer_selection, &timer_capabilities) &&
                   timer_capabilities.pit_legacy_available != 0U;
        }
    }
    return 0;
}

static int pulse_context_is_valid(const DAWN_CONTEXT *context) {
    DAWN_ACPI_ROOT_TABLE_METADATA root_table;

    return context != (void *)0 && context->magic == DAWN_CONTEXT_MAGIC &&
           context->version == DAWN_CONTEXT_VERSION && context->size >= sizeof(DAWN_CONTEXT) &&
           context->memory_map_physical_address != 0 && context->memory_map_size != 0 &&
           context->memory_descriptor_size == sizeof(DAWN_MEMORY_DESCRIPTOR) &&
           context->memory_descriptor_version == DAWN_MEMORY_DESCRIPTOR_VERSION && context->kernel_stack_top != 0 &&
           context->kernel_stack_size >= 4096U && context->framebuffer_physical_address != 0U &&
           context->framebuffer_byte_size != 0U && context->framebuffer_width != 0U &&
           context->framebuffer_height != 0U && context->framebuffer_pixels_per_scan_line >= context->framebuffer_width &&
           context->boot_reservations_physical_address != 0U && context->boot_reservations_size != 0U &&
           context->boot_reservation_descriptor_size == sizeof(DAWN_MEMORY_RANGE) &&
           context->boot_reservation_descriptor_version == DAWN_MEMORY_RANGE_VERSION &&
           context->boot_reservation_count != 0U &&
           context->boot_reservations_size ==
               (uint64_t)context->boot_reservation_count * context->boot_reservation_descriptor_size &&
           context->acpi_rsdp_physical_address != 0U &&
           dawn_acpi_rsdp_is_valid((const void *)(uintptr_t)context->acpi_rsdp_physical_address) &&
           dawn_acpi_root_table_describe((const void *)(uintptr_t)context->acpi_rsdp_physical_address, &root_table) &&
           (context->framebuffer_pixel_format == DAWN_PIXEL_FORMAT_RGBX8888 ||
            context->framebuffer_pixel_format == DAWN_PIXEL_FORMAT_BGRX8888 ||
            context->framebuffer_pixel_format == DAWN_PIXEL_FORMAT_BGR888);
}

__attribute__((noreturn)) void pulse_entry(const DAWN_CONTEXT *context) {
    uint64_t first_frame;
    uint64_t second_frame;
    uint32_t first_task;
    uint32_t second_task;
    uint32_t selected_task;

    if (pulse_context_is_valid(context) && pulse_memory_initialize(context) &&
        pulse_memory_take_frame(&first_frame) && pulse_memory_take_frame(&second_frame) &&
        second_frame == first_frame + 4096U && pulse_paging_initialize() &&
        pulse_interrupts_initialize() && prism_canvas_runtime_probe(context) && horizon_runtime_probe() &&
        horizon_input_runtime_probe() && origin_runtime_probe() && vaultfs_runtime_probe() &&
        vibe_session_runtime_probe() && parcel_runtime_probe() && atlas_keyboard_runtime_probe() && atlas_pci_runtime_probe() &&
        pulse_acpi_child_table_inventory_probe(context)) {
        pulse_debug_write("ORIGIN: delegated key verified\n");
        pulse_debug_write("VAULT: redundant superblock recovered\n");
        pulse_debug_write("VAULT: journal commit verified\n");
        pulse_debug_write("VAULT: A/B slot state verified\n");
        pulse_debug_write("SESSION: launch policy verified\n");
        pulse_debug_write("PARCEL: signed manifest policy verified\n");
        pulse_debug_write("PARCEL: native launch admission verified\n");
        pulse_debug_write("PRISM: framebuffer painted\n");
        pulse_debug_write("CANVAS: retained scene rendered\n");
        pulse_debug_write("HORIZON: desktop scene rendered\n");
        pulse_debug_write("HORIZON: desktop focus model verified\n");
        pulse_debug_write("HORIZON: keyboard focus adapter verified\n");
        pulse_debug_write("ATLAS: keyboard event queue verified\n");
        pulse_debug_write("ATLAS: PCI inventory verified\n");
        pulse_debug_write("ATLAS: PCI resource inventory verified\n");
        pulse_debug_write("DAWN: ACPI RSDP handoff verified\n");
        pulse_debug_write("DAWN: ACPI root table metadata verified\n");
        pulse_debug_write("DAWN: ACPI child table inventory verified\n");
        pulse_debug_write("DAWN: ACPI MADT metadata inventory verified\n");
        pulse_debug_write("DAWN: ACPI x86 APIC metadata inventory verified\n");
        pulse_debug_write("PULSE: PIC controller policy verified\n");
        pulse_debug_write("PULSE: x86 APIC handoff plan verified\n");
        pulse_debug_write("PULSE: PIT timer source policy verified\n");
        pulse_debug_write("PULSE: timer capability inventory verified\n");
        pulse_scheduler_initialize();
        if (!pulse_scheduler_create_ready_task(&first_task) ||
            !pulse_scheduler_create_ready_task(&second_task) ||
            !pulse_scheduler_select_next(&selected_task) || selected_task != first_task ||
            !pulse_scheduler_select_next(&selected_task) || selected_task != second_task) {
            pulse_debug_write("PULSE: early scheduler bootstrap failed\n");
            pulse_debug_exit();
        }
#if defined(PULSE_PROBE_panic)
        __asm__ volatile("ud2");
        pulse_debug_write("PULSE: invalid opcode unexpectedly returned\n");
#elif defined(PULSE_PROBE_keyboard)
        PRISM_FRAMEBUFFER framebuffer;
        HORIZON_DESKTOP_RUNTIME desktop_runtime;
        HORIZON_DESKTOP_RUNTIME_STEP_RESULT desktop_step;
        PARCEL_NATIVE_LAUNCH_REQUEST launch_request;
        uint32_t keyboard_redraw_count = 0U;

        if (!prism_framebuffer_from_dawn(context, &framebuffer) ||
            !horizon_desktop_runtime_initialize(&framebuffer, &desktop_runtime)) {
            pulse_debug_write("HORIZON: keyboard desktop initialization failed\n");
            pulse_debug_exit();
        }
        if (!atlas_i8042_keyboard_prepare_irq1()) {
            pulse_debug_write("ATLAS: keyboard IRQ1 preparation failed\n");
            pulse_debug_exit();
        }
        pulse_debug_write("ATLAS: keyboard irq probe ready\n");
        __asm__ volatile("sti" : : : "memory");
        for (;;) {
            __asm__ volatile("hlt");
            if (!horizon_desktop_runtime_step(&desktop_runtime, HORIZON_INPUT_PUMP_MAX_EVENTS, &desktop_step)) {
                pulse_debug_write("HORIZON: keyboard event pump failed\n");
                pulse_debug_exit();
            }
            if (desktop_step.selected_application != (const void *)0) {
                if (!parcel_native_launch_request_initialize(
                        &launch_request, desktop_step.selected_application->id)) {
                    pulse_debug_write("PARCEL: native launch request formation failed\n");
                    pulse_debug_exit();
                }
                pulse_debug_write("PARCEL: native launch request formed\n");
            }
            if (desktop_step.redraw_performed != 0U) {
                ++keyboard_redraw_count;
                if (keyboard_redraw_count == 1U) {
                    pulse_debug_write("HORIZON: keyboard focus redrawn\n");
                } else if (keyboard_redraw_count == 2U) {
                    pulse_debug_write("HORIZON: keyboard selection redrawn\n");
                    pulse_debug_exit();
                }
            }
        }
#else
        if (!pulse_context_run_probe()) {
            pulse_debug_write("PULSE: task context bootstrap failed\n");
            pulse_debug_exit();
        }
        pulse_timer_start_probe();
        for (;;) {
            __asm__ volatile("hlt");
        }
#endif
    } else {
        pulse_debug_write("PULSE: early interrupt bootstrap failed\n");
    }

    pulse_debug_exit();

    for (;;) {
        __asm__ volatile("hlt");
    }
}
