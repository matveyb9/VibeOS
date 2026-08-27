/* VibeOS Parcel — no-libc bootstrap manifest and capability installation gate. */

#include "parcel.h"

#include <horizon.h>

static int parcel_identifier_valid(const char *identifier) {
    uint32_t index;
    char character;

    if (identifier == (void *)0 || identifier[0] == '\0') {
        return 0;
    }
    for (index = 0; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        character = identifier[index];
        if (character == '\0') {
            return 1;
        }
        if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
              character == '.' || character == '-')) {
            return 0;
        }
    }
    return 0;
}

static void parcel_copy_bytes(uint8_t *destination, const uint8_t *source, uint64_t count) {
    uint64_t index;

    for (index = 0; index < count; ++index) {
        destination[index] = source[index];
    }
}

static int parcel_identifier_equal(const char *left, const char *right) {
    uint32_t index;

    for (index = 0; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        if (left[index] != right[index]) {
            return 0;
        }
        if (left[index] == '\0') {
            return 1;
        }
    }
    return 0;
}

static const char *parcel_native_application_identifier(uint32_t native_application_id) {
    if (native_application_id == HORIZON_APPLICATION_PROMPT) {
        return "org.vibe.prompt";
    }
    if (native_application_id == HORIZON_APPLICATION_CUE) {
        return "org.vibe.cue";
    }
    if (native_application_id == HORIZON_APPLICATION_VECTOR) {
        return "org.vibe.vector";
    }
    return (const void *)0;
}

void parcel_manifest_initialize(
    PARCEL_MANIFEST *manifest,
    const char *application_id,
    PARCEL_SCOPE scope,
    uint64_t payload_bytes,
    uint32_t payload_checksum,
    uint32_t requested_rights) {
    uint32_t index;

    if (manifest == (void *)0) {
        return;
    }
    manifest->format_version = PARCEL_VPK_FORMAT_VERSION;
    manifest->scope = (uint32_t)scope;
    for (index = 0; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        manifest->application_id[index] = application_id != (void *)0 ? application_id[index] : '\0';
        if (manifest->application_id[index] == '\0') {
            ++index;
            break;
        }
    }
    for (; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        manifest->application_id[index] = '\0';
    }
    manifest->payload_bytes = payload_bytes;
    manifest->payload_checksum = payload_checksum;
    manifest->requested_rights = requested_rights;
}

int parcel_manifest_valid(const PARCEL_MANIFEST *manifest) {
    uint32_t allowed_rights = VIBE_RIGHT_READ | VIBE_RIGHT_WRITE | VIBE_RIGHT_INSPECT;

    return manifest != (void *)0 && manifest->format_version == PARCEL_VPK_FORMAT_VERSION &&
           (manifest->scope == PARCEL_SCOPE_CORE || manifest->scope == PARCEL_SCOPE_LOCAL ||
            manifest->scope == PARCEL_SCOPE_USER) &&
           parcel_identifier_valid(manifest->application_id) && manifest->payload_bytes != 0U &&
           manifest->requested_rights != 0U && (manifest->requested_rights & ~allowed_rights) == 0U;
}

void parcel_registry_initialize(PARCEL_REGISTRY *registry) {
    uint32_t index;
    uint8_t *bytes;

    if (registry != (void *)0) {
        bytes = (uint8_t *)registry;
        for (index = 0; index < sizeof(*registry); ++index) {
            bytes[index] = 0;
        }
    }
}

int parcel_registry_install(
    PARCEL_REGISTRY *registry,
    VIBE_KEY installer_key,
    const PARCEL_INSTALL_REQUEST *request) {
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights;
    uint32_t index;

    if (registry == (void *)0 || request == (void *)0 || request->signature_verified == 0U ||
        !parcel_manifest_valid(&request->manifest) || registry->count >= PARCEL_REGISTRY_CAPACITY ||
        !origin_key_inspect(installer_key, &object_id, &rights) || object_id != PARCEL_REGISTRY_OBJECT ||
        (rights & VIBE_RIGHT_WRITE) == 0U) {
        return 0;
    }
    for (index = 0; index < registry->count; ++index) {
        if (parcel_identifier_equal(registry->entries[index].application_id, request->manifest.application_id)) {
            return 0;
        }
    }
    parcel_copy_bytes(
        (uint8_t *)&registry->entries[registry->count],
        (const uint8_t *)&request->manifest,
        sizeof(PARCEL_MANIFEST));
    ++registry->count;
    return 1;
}

int parcel_native_launch_request_initialize(
    PARCEL_NATIVE_LAUNCH_REQUEST *request, uint32_t native_application_id) {
    const char *application_id;
    uint32_t index;

    if (request == (void *)0 ||
        (application_id = parcel_native_application_identifier(native_application_id)) == (const void *)0) {
        return 0;
    }
    request->format_version = PARCEL_NATIVE_LAUNCH_REQUEST_VERSION;
    request->native_application_id = native_application_id;
    for (index = 0U; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        request->application_id[index] = application_id[index];
        if (application_id[index] == '\0') {
            ++index;
            break;
        }
    }
    for (; index < PARCEL_APPLICATION_ID_BYTES; ++index) {
        request->application_id[index] = '\0';
    }
    return 1;
}

int parcel_native_launch_request_valid(const PARCEL_NATIVE_LAUNCH_REQUEST *request) {
    const char *application_id;

    return request != (const void *)0 && request->format_version == PARCEL_NATIVE_LAUNCH_REQUEST_VERSION &&
           (application_id = parcel_native_application_identifier(request->native_application_id)) != (const void *)0 &&
           parcel_identifier_equal(request->application_id, application_id);
}

int parcel_executable_image_descriptor_valid(const PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR *descriptor) {
    return descriptor != (const void *)0 && descriptor->format_version == PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR_VERSION &&
           descriptor->image_format == PARCEL_EXECUTABLE_IMAGE_FORMAT_FLAT_X86_64 && descriptor->byte_count != 0U &&
           descriptor->entry_offset < descriptor->byte_count && descriptor->payload_checksum != 0U;
}

static uint16_t parcel_read_u16_le(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8U));
}

static uint32_t parcel_read_u32_le(const uint8_t *bytes) {
    return (uint32_t)((uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
                      ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U));
}

static uint64_t parcel_read_u64_le(const uint8_t *bytes) {
    uint64_t value = 0U;
    uint32_t index;

    for (index = 0U; index < 8U; ++index) {
        value |= ((uint64_t)bytes[index]) << (index * 8U);
    }
    return value;
}

int parcel_elf64_header_describe(
    const uint8_t *header,
    uint32_t header_bytes,
    PARCEL_ELF64_HEADER_METADATA *metadata) {
    uint16_t program_header_entry_size;
    uint16_t program_header_count;
    uint64_t program_header_offset;

    if (header == (const void *)0 || metadata == (void *)0 || header_bytes < PARCEL_ELF64_HEADER_BYTES ||
        header[0] != 0x7fU || header[1] != 'E' || header[2] != 'L' || header[3] != 'F' || header[4] != 2U ||
        header[5] != 1U || header[6] != 1U || parcel_read_u16_le(&header[16]) != 2U ||
        parcel_read_u16_le(&header[18]) != 62U || header[20] != 1U || header[21] != 0U || header[22] != 0U ||
        header[23] != 0U ||
        parcel_read_u16_le(&header[52]) != PARCEL_ELF64_HEADER_BYTES) {
        return 0;
    }
    program_header_offset = parcel_read_u64_le(&header[32]);
    program_header_entry_size = parcel_read_u16_le(&header[54]);
    program_header_count = parcel_read_u16_le(&header[56]);
    if ((program_header_count == 0U &&
         (program_header_offset != 0U || program_header_entry_size != 0U)) ||
        (program_header_count != 0U &&
         (program_header_offset < PARCEL_ELF64_HEADER_BYTES || program_header_entry_size < 56U ||
          program_header_offset > UINT64_MAX -
                                      ((uint64_t)program_header_entry_size * (uint64_t)program_header_count)))) {
        return 0;
    }
    metadata->image_type = parcel_read_u16_le(&header[16]);
    metadata->machine = parcel_read_u16_le(&header[18]);
    metadata->entry_address = parcel_read_u64_le(&header[24]);
    metadata->program_header_offset = program_header_offset;
    metadata->header_size = parcel_read_u16_le(&header[52]);
    metadata->program_header_entry_size = program_header_entry_size;
    metadata->program_header_count = program_header_count;
    return 1;
}

int parcel_elf64_program_headers_describe(
    const PARCEL_ELF64_HEADER_METADATA *header_metadata,
    const uint8_t *program_headers,
    uint32_t program_header_bytes,
    PARCEL_ELF64_PROGRAM_HEADER_METADATA *metadata) {
    uint32_t index;
    uint64_t virtual_address;
    uint64_t file_bytes;
    uint64_t memory_bytes;
    uint64_t virtual_end;
    uint64_t alignment;

    if (header_metadata == (const void *)0 || program_headers == (const void *)0 || metadata == (void *)0 ||
        header_metadata->image_type != 2U || header_metadata->machine != 62U ||
        header_metadata->program_header_entry_size != PARCEL_ELF64_PROGRAM_HEADER_BYTES ||
        header_metadata->program_header_count == 0U ||
        program_header_bytes < (uint32_t)header_metadata->program_header_count * PARCEL_ELF64_PROGRAM_HEADER_BYTES) {
        return 0;
    }
    metadata->loadable_segment_count = 0U;
    metadata->lowest_virtual_address = UINT64_MAX;
    metadata->highest_virtual_address = 0U;
    for (index = 0U; index < header_metadata->program_header_count; ++index) {
        const uint8_t *entry = &program_headers[index * PARCEL_ELF64_PROGRAM_HEADER_BYTES];

        if (parcel_read_u32_le(&entry[0]) != 1U || (parcel_read_u32_le(&entry[4]) & ~UINT32_C(7)) != 0U) {
            return 0;
        }
        virtual_address = parcel_read_u64_le(&entry[16]);
        file_bytes = parcel_read_u64_le(&entry[32]);
        memory_bytes = parcel_read_u64_le(&entry[40]);
        alignment = parcel_read_u64_le(&entry[48]);
        if (file_bytes > memory_bytes || virtual_address > UINT64_MAX - memory_bytes ||
            (alignment > 1U && ((alignment & (alignment - 1U)) != 0U ||
                                (parcel_read_u64_le(&entry[8]) % alignment) != (virtual_address % alignment)))) {
            return 0;
        }
        virtual_end = virtual_address + memory_bytes;
        if (virtual_address < metadata->lowest_virtual_address) {
            metadata->lowest_virtual_address = virtual_address;
        }
        if (virtual_end > metadata->highest_virtual_address) {
            metadata->highest_virtual_address = virtual_end;
        }
        ++metadata->loadable_segment_count;
    }
    return metadata->loadable_segment_count != 0U;
}

int parcel_registry_admits_native_launch(
    const PARCEL_REGISTRY *registry, const PARCEL_NATIVE_LAUNCH_REQUEST *request) {
    PARCEL_NATIVE_LAUNCH_ADMISSION admission;

    return parcel_registry_admit_native_launch(registry, request, &admission) &&
           admission.status == PARCEL_NATIVE_LAUNCH_ADMITTED;
}

int parcel_registry_admit_native_launch(
    const PARCEL_REGISTRY *registry,
    const PARCEL_NATIVE_LAUNCH_REQUEST *request,
    PARCEL_NATIVE_LAUNCH_ADMISSION *admission) {
    uint32_t index;

    if (admission == (void *)0) {
        return 0;
    }
    admission->native_application_id = request != (const void *)0 ? request->native_application_id : 0U;
    admission->status = PARCEL_NATIVE_LAUNCH_REJECTED_INVALID_REQUEST;
    if (registry == (const void *)0 || !parcel_native_launch_request_valid(request)) {
        return 1;
    }
    for (index = 0U; index < registry->count; ++index) {
        if (parcel_identifier_equal(registry->entries[index].application_id, request->application_id)) {
            admission->status = PARCEL_NATIVE_LAUNCH_ADMITTED;
            return 1;
        }
    }
    admission->status = PARCEL_NATIVE_LAUNCH_REJECTED_NOT_INSTALLED;
    return 1;
}

int parcel_runtime_probe(void) {
    PARCEL_MANIFEST manifest;
    PARCEL_INSTALL_REQUEST request;
    PARCEL_REGISTRY registry;
    PARCEL_NATIVE_LAUNCH_REQUEST launch_request;
    VIBE_KEY installer_key;

    origin_keys_reset();
    parcel_manifest_initialize(
        &manifest,
        "org.vibe.prompt",
        PARCEL_SCOPE_CORE,
        UINT64_C(4096),
        UINT32_C(0x89abcdef),
        VIBE_RIGHT_READ | VIBE_RIGHT_INSPECT);
    parcel_copy_bytes(
        (uint8_t *)&request.manifest,
        (const uint8_t *)&manifest,
        sizeof(PARCEL_MANIFEST));
    request.signature_verified = 1;
    parcel_registry_initialize(&registry);
    return origin_key_mint(PARCEL_REGISTRY_OBJECT, VIBE_RIGHT_WRITE, &installer_key) &&
           parcel_registry_install(&registry, installer_key, &request) && registry.count == 1U &&
           parcel_native_launch_request_initialize(&launch_request, HORIZON_APPLICATION_PROMPT) &&
           parcel_registry_admits_native_launch(&registry, &launch_request);
}
