/* VibeOS Parcel — bootstrap VPK manifest and installation policy. */

#ifndef VIBEOS_PARCEL_H
#define VIBEOS_PARCEL_H

#include <keys.h>

#define PARCEL_VPK_FORMAT_VERSION UINT32_C(1)
#define PARCEL_APPLICATION_ID_BYTES UINT32_C(32)
#define PARCEL_REGISTRY_CAPACITY UINT32_C(16)
#define PARCEL_REGISTRY_OBJECT UINT64_C(2)
#define PARCEL_NATIVE_LAUNCH_REQUEST_VERSION UINT32_C(1)
#define PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR_VERSION UINT32_C(1)
#define PARCEL_ELF64_HEADER_BYTES UINT32_C(64)

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

typedef struct {
    uint32_t format_version;
    uint32_t native_application_id;
    char application_id[PARCEL_APPLICATION_ID_BYTES];
} PARCEL_NATIVE_LAUNCH_REQUEST;

typedef enum {
    PARCEL_NATIVE_LAUNCH_REJECTED_INVALID_REQUEST = 1,
    PARCEL_NATIVE_LAUNCH_REJECTED_NOT_INSTALLED = 2,
    PARCEL_NATIVE_LAUNCH_ADMITTED = 3
} PARCEL_NATIVE_LAUNCH_ADMISSION_STATUS;

typedef struct {
    uint32_t status;
    uint32_t native_application_id;
} PARCEL_NATIVE_LAUNCH_ADMISSION;

typedef enum {
    PARCEL_EXECUTABLE_IMAGE_FORMAT_FLAT_X86_64 = 1
} PARCEL_EXECUTABLE_IMAGE_FORMAT;

typedef struct {
    uint32_t format_version;
    uint32_t image_format;
    uint64_t byte_count;
    uint64_t entry_offset;
    uint32_t payload_checksum;
} PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR;

typedef struct {
    uint16_t image_type;
    uint16_t machine;
    uint64_t entry_address;
    uint64_t program_header_offset;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_count;
} PARCEL_ELF64_HEADER_METADATA;

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
int parcel_native_launch_request_initialize(
    PARCEL_NATIVE_LAUNCH_REQUEST *request, uint32_t native_application_id);
int parcel_native_launch_request_valid(const PARCEL_NATIVE_LAUNCH_REQUEST *request);
int parcel_registry_admits_native_launch(
    const PARCEL_REGISTRY *registry, const PARCEL_NATIVE_LAUNCH_REQUEST *request);
int parcel_registry_admit_native_launch(
    const PARCEL_REGISTRY *registry,
    const PARCEL_NATIVE_LAUNCH_REQUEST *request,
    PARCEL_NATIVE_LAUNCH_ADMISSION *admission);
int parcel_executable_image_descriptor_valid(const PARCEL_EXECUTABLE_IMAGE_DESCRIPTOR *descriptor);
int parcel_elf64_header_describe(
    const uint8_t *header,
    uint32_t header_bytes,
    PARCEL_ELF64_HEADER_METADATA *metadata);
int parcel_runtime_probe(void);

#endif
