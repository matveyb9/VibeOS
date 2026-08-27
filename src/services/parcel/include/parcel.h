/* VibeOS Parcel — bootstrap VPK manifest and installation policy. */

#ifndef VIBEOS_PARCEL_H
#define VIBEOS_PARCEL_H

#include <keys.h>

#define PARCEL_VPK_FORMAT_VERSION UINT32_C(1)
#define PARCEL_APPLICATION_ID_BYTES UINT32_C(32)
#define PARCEL_REGISTRY_CAPACITY UINT32_C(16)
#define PARCEL_REGISTRY_OBJECT UINT64_C(2)

typedef enum {
    PARCEL_SCOPE_CORE = 1,
    PARCEL_SCOPE_LOCAL = 2,
    PARCEL_SCOPE_USER = 3
} PARCEL_SCOPE;

typedef struct {
    uint32_t format_version;
    uint32_t scope;
    char application_id[PARCEL_APPLICATION_ID_BYTES];
    uint64_t payload_bytes;
    uint32_t payload_checksum;
    uint32_t requested_rights;
} PARCEL_MANIFEST;

typedef struct {
    PARCEL_MANIFEST manifest;
    uint32_t signature_verified;
} PARCEL_INSTALL_REQUEST;

typedef struct {
    PARCEL_MANIFEST entries[PARCEL_REGISTRY_CAPACITY];
    uint32_t count;
} PARCEL_REGISTRY;

void parcel_manifest_initialize(
    PARCEL_MANIFEST *manifest,
    const char *application_id,
    PARCEL_SCOPE scope,
    uint64_t payload_bytes,
    uint32_t payload_checksum,
    uint32_t requested_rights);
int parcel_manifest_valid(const PARCEL_MANIFEST *manifest);
void parcel_registry_initialize(PARCEL_REGISTRY *registry);
int parcel_registry_install(
    PARCEL_REGISTRY *registry,
    VIBE_KEY installer_key,
    const PARCEL_INSTALL_REQUEST *request);
int parcel_runtime_probe(void);

#endif
