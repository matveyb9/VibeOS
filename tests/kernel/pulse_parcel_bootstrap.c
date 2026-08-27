/* VibeOS Parcel — host checks for VPK manifest and Key-gated install policy. */

#include <stdio.h>

#include <keys.h>
#include <horizon.h>
#include <parcel.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    PARCEL_MANIFEST manifest;
    PARCEL_INSTALL_REQUEST request;
    PARCEL_REGISTRY registry;
    PARCEL_NATIVE_LAUNCH_REQUEST launch_request;
    PARCEL_NATIVE_LAUNCH_ADMISSION admission;
    PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR image = {
        PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR_VERSION,
        PARCEL_EXECUTABLE_IMAGE_FORMAT_FLAT_X86_64,
        UINT64_C(4096),
        UINT64_C(64),
        UINT32_C(0x11223344),
    };
    uint8_t elf_header[PARCEL_ELF64_HEADER_BYTES] = {0};
    PARCEL_ELF64_HEADER_METADATA elf_metadata;
    uint8_t elf_program_header[PARCEL_ELF64_PROGRAM_HEADER_BYTES] = {0};
    PARCEL_ELF64_PROGRAM_HEADER_METADATA program_header_metadata;
    PARCEL_ELF64_LOAD_PLAN load_plan;
    PARCEL_ELF64_HEADER_METADATA invalid_header_metadata;
    VIBE_KEY authority;
    VIBE_KEY read_only;

    origin_keys_reset();
    elf_header[0] = 0x7fU;
    elf_header[1] = 'E';
    elf_header[2] = 'L';
    elf_header[3] = 'F';
    elf_header[4] = 2U;
    elf_header[5] = 1U;
    elf_header[6] = 1U;
    elf_header[16] = 2U;
    elf_header[18] = 62U;
    elf_header[20] = 1U;
    elf_header[24] = 0x44U;
    elf_header[25] = 0x40U;
    elf_header[26] = 0x40U;
    elf_header[32] = PARCEL_ELF64_HEADER_BYTES;
    elf_header[52] = PARCEL_ELF64_HEADER_BYTES;
    elf_header[54] = 56U;
    elf_header[56] = 1U;
    elf_program_header[0] = 1U;
    elf_program_header[4] = 5U;
    elf_program_header[8] = 0x40U;
    elf_program_header[16] = 0x40U;
    elf_program_header[17] = 0x40U;
    elf_program_header[18] = 0x40U;
    elf_program_header[32] = 16U;
    elf_program_header[40] = 16U;
    parcel_manifest_initialize(
        &manifest,
        "org.vibe.cue",
        PARCEL_SCOPE_LOCAL,
        UINT64_C(8192),
        UINT32_C(0x11223344),
        VIBE_RIGHT_READ);
    request.manifest = manifest;
    request.signature_verified = 1;
    parcel_registry_initialize(&registry);

    elf_header[0] = 0U;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "invalid ELF magic is rejected")) {
        return 1;
    }
    elf_header[0] = 0x7fU;
    elf_header[5] = 2U;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "non-little-endian ELF header is rejected")) {
        return 1;
    }
    elf_header[5] = 1U;
    elf_header[18] = 0U;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "non-x86-64 ELF header is rejected")) {
        return 1;
    }
    elf_header[18] = 62U;
    elf_header[52] = 0U;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "wrong ELF header size is rejected")) {
        return 1;
    }
    elf_header[52] = PARCEL_ELF64_HEADER_BYTES;
    elf_header[54] = 55U;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "short program header entries are rejected")) {
        return 1;
    }
    elf_header[54] = 56U;
    elf_header[32] = 0xffU;
    elf_header[33] = 0xffU;
    elf_header[34] = 0xffU;
    elf_header[35] = 0xffU;
    elf_header[36] = 0xffU;
    elf_header[37] = 0xffU;
    elf_header[38] = 0xffU;
    elf_header[39] = 0xffU;
    if (!expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "program header extent overflow is rejected")) {
        return 1;
    }
    elf_header[32] = PARCEL_ELF64_HEADER_BYTES;
    elf_header[33] = 0U;
    elf_header[34] = 0U;
    elf_header[35] = 0U;
    elf_header[36] = 0U;
    elf_header[37] = 0U;
    elf_header[38] = 0U;
    elf_header[39] = 0U;

    if (!expect(parcel_manifest_valid(&manifest), "valid manifest is accepted") ||
        !expect(parcel_executable_image_descriptor_valid(&image), "bounded image descriptor is valid") ||
        !expect(!parcel_elf64_header_describe(elf_header, PARCEL_ELF64_HEADER_BYTES - 1U, &elf_metadata),
                    "short bounded ELF input is rejected") ||
        !expect(parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata) &&
                    elf_metadata.image_type == 2U && elf_metadata.machine == 62U &&
                    elf_metadata.entry_address == UINT64_C(0x404044) && elf_metadata.program_header_offset == 64U &&
                    elf_metadata.program_header_entry_size == 56U && elf_metadata.program_header_count == 1U &&
                    elf_metadata.header_size == PARCEL_ELF64_HEADER_BYTES,
                    "bounded ELF64 header metadata is described") ||
        !expect(parcel_elf64_program_headers_describe(
                    &elf_metadata, elf_program_header, sizeof(elf_program_header), &program_header_metadata) &&
                    program_header_metadata.loadable_segment_count == 1U &&
                    program_header_metadata.lowest_virtual_address == UINT64_C(0x404040) &&
                    program_header_metadata.highest_virtual_address == UINT64_C(0x404050),
                    "bounded ELF64 loadable program-header metadata is described") ||
        !expect(parcel_elf64_load_plan_form(&elf_metadata, &program_header_metadata, &load_plan) &&
                    load_plan.entry_address == UINT64_C(0x404044) &&
                    load_plan.virtual_address_start == UINT64_C(0x404040) &&
                    load_plan.virtual_address_end == UINT64_C(0x404050) && load_plan.loadable_segment_count == 1U,
                    "immutable ELF64 load-plan metadata is formed") ||
        !expect(origin_key_mint(PARCEL_REGISTRY_OBJECT, VIBE_RIGHT_READ | VIBE_RIGHT_WRITE, &authority),
                    "registry authority is minted") ||
        !expect(origin_key_narrow(authority, VIBE_RIGHT_READ, &read_only), "read-only child is minted") ||
        !expect(!parcel_registry_install(&registry, read_only, &request),
                    "read-only key cannot install") ||
        !expect(parcel_registry_install(&registry, authority, &request),
                    "write key installs verified manifest") ||
        !expect(registry.count == 1U, "registry stores installation") ||
        !expect(parcel_native_launch_request_initialize(&launch_request, HORIZON_APPLICATION_CUE),
                    "known native application forms a request") ||
        !expect(parcel_native_launch_request_valid(&launch_request), "native launch request is valid") ||
        !expect(parcel_registry_admit_native_launch(&registry, &launch_request, &admission) &&
                    admission.status == PARCEL_NATIVE_LAUNCH_ADMITTED &&
                    admission.native_application_id == HORIZON_APPLICATION_CUE,
                    "installed native application has an explicit admitted result") ||
        !expect(parcel_registry_admits_native_launch(&registry, &launch_request),
                    "installed native application is admitted") ||
        !expect(!parcel_native_launch_request_initialize(&launch_request, UINT32_C(99)),
                    "unknown native application is rejected") ||
        !expect(!parcel_registry_install(&registry, authority, &request),
                    "duplicate application identifier is rejected")) {
        return 1;
    }

    invalid_header_metadata = elf_metadata;
    invalid_header_metadata.entry_address = program_header_metadata.highest_virtual_address;
    if (!expect(!parcel_elf64_load_plan_form(&invalid_header_metadata, &program_header_metadata, &load_plan),
                    "load plan rejects an entry outside declared virtual span")) {
        return 1;
    }

    request.signature_verified = 0;
    image.entry_offset = image.byte_count;
    elf_header[4] = 1U;
    elf_program_header[0] = 2U;
    if (!expect(!parcel_registry_install(&registry, authority, &request),
                    "unverified manifest is rejected") ||
        !expect(!parcel_executable_image_descriptor_valid(&image), "out-of-range image entry offset is rejected") ||
        !expect(!parcel_elf64_header_describe(elf_header, sizeof(elf_header), &elf_metadata),
                    "non-ELF64 header is rejected") ||
        !expect(!parcel_elf64_program_headers_describe(
                    &elf_metadata, elf_program_header, sizeof(elf_program_header), &program_header_metadata),
                    "non-loadable program header is rejected") ||
        !expect(parcel_native_launch_request_initialize(&launch_request, HORIZON_APPLICATION_VECTOR) &&
                    parcel_registry_admit_native_launch(&registry, &launch_request, &admission) &&
                    admission.status == PARCEL_NATIVE_LAUNCH_REJECTED_NOT_INSTALLED,
                    "uninstalled native application has an explicit rejection result") ||
        !expect(parcel_registry_admit_native_launch(&registry, (const void *)0, &admission) &&
                    admission.status == PARCEL_NATIVE_LAUNCH_REJECTED_INVALID_REQUEST,
                    "invalid request has an explicit rejection result") ||
        !expect(parcel_runtime_probe(), "Parcel runtime probe succeeds")) {
        return 1;
    }

    puts("Parcel bootstrap unit tests passed.");
    return 0;
}
