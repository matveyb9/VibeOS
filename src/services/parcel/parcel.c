/* VibeOS Parcel — no-libc bootstrap manifest and capability installation gate. */

#include "parcel.h"

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

int parcel_runtime_probe(void) {
    PARCEL_MANIFEST manifest;
    PARCEL_INSTALL_REQUEST request;
    PARCEL_REGISTRY registry;
    VIBE_KEY installer_key;

    origin_keys_reset();
    parcel_manifest_initialize(
        &manifest,
        "org.vibe.guide",
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
           parcel_registry_install(&registry, installer_key, &request) && registry.count == 1U;
}
