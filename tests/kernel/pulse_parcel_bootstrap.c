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
    VIBE_KEY authority;
    VIBE_KEY read_only;

    origin_keys_reset();
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

    if (!expect(parcel_manifest_valid(&manifest), "valid manifest is accepted") ||
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
        !expect(parcel_registry_admits_native_launch(&registry, &launch_request),
                    "installed native application is admitted") ||
        !expect(!parcel_native_launch_request_initialize(&launch_request, UINT32_C(99)),
                    "unknown native application is rejected") ||
        !expect(!parcel_registry_install(&registry, authority, &request),
                    "duplicate application identifier is rejected")) {
        return 1;
    }

    request.signature_verified = 0;
    if (!expect(!parcel_registry_install(&registry, authority, &request),
                    "unverified manifest is rejected") ||
        !expect(parcel_runtime_probe(), "Parcel runtime probe succeeds")) {
        return 1;
    }

    puts("Parcel bootstrap unit tests passed.");
    return 0;
}
