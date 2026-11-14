#pragma once

namespace csms {

/**
 * Single source of truth for the CSMS application identity and version.
 *
 * All components that need the application name or version (controllers,
 * health endpoint, UI navigation, tests) must include this header and
 * reference these constants — never hardcode the values elsewhere.
 *
 * This is a header-only constexpr definition, so the version is resolved
 * at compile time with zero runtime overhead.
 */
constexpr const char* APP_NAME       = "Community Services Management System";
constexpr const char* APP_SHORT_NAME  = "CSMS";
constexpr const char* APP_VERSION      = "1.0.0";

} // namespace csms
