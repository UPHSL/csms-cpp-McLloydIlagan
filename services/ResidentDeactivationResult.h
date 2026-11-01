#pragma once

#include <optional>

#include "../models/Resident.h"

namespace csms {

/**
 * Represents the outcome of one Resident deactivation attempt.
 *
 * Three distinct outcomes are possible:
 *
 *   Successfully deactivated (Active -> Inactive):
 *     success        = true
 *     alreadyInactive = false
 *     notFound       = false
 *     resident       = persisted Resident with status Inactive
 *
 *   Already Inactive (safe repeated operation):
 *     success        = true
 *     alreadyInactive = true
 *     notFound       = false
 *     resident       = persisted Resident (still Inactive)
 *
 *   Resident not found:
 *     success        = false
 *     alreadyInactive = false
 *     notFound       = true
 *     resident       = std::nullopt
 */
struct ResidentDeactivationResult
{
    bool success{false};
    bool alreadyInactive{false};
    bool notFound{false};

    std::optional<Resident> resident{std::nullopt};
};

} // namespace csms
