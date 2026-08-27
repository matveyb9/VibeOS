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
        vibe_session_runtime_probe() && parcel_runtime_probe() && atlas_keyboard_runtime_probe() && atlas_pci_runtime_probe()) {
        pulse_debug_write("ORIGIN: delegated key verified\n");
        pulse_debug_write("VAULT: redundant superblock recovered\n");
        pulse_debug_write("VAULT: journal commit verified\n");
        pulse_debug_write("VAULT: A/B slot state verified\n");
        pulse_debug_write("SESSION: launch policy verified\n");
        pulse_debug_write("PARCEL: signed manifest policy verified\n");
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
        HORIZON_DESKTOP_STATE desktop_state;
        HORIZON_INPUT_PUMP_RESULT input_result;
        PRISM_FRAMEBUFFER framebuffer;

        if (!prism_framebuffer_from_dawn(context, &framebuffer) ||
            !horizon_desktop_state_initialize(&desktop_state, 3U) ||
            !horizon_render_desktop_for_state(&framebuffer, &desktop_state)) {
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
            if (!horizon_input_pump(&desktop_state, HORIZON_INPUT_PUMP_MAX_EVENTS, &input_result)) {
                pulse_debug_write("HORIZON: keyboard event pump failed\n");
                pulse_debug_exit();
            }
            if (input_result.redraw_requested != 0U) {
                if (!horizon_render_desktop_for_state(&framebuffer, &desktop_state)) {
                    pulse_debug_write("HORIZON: keyboard focus redraw failed\n");
                    pulse_debug_exit();
                }
                pulse_debug_write("HORIZON: keyboard focus redrawn\n");
                pulse_debug_exit();
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
