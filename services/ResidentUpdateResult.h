#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/Resident.h"

namespace csms {

/**
 * Represents the outcome of one Resident update attempt.
 *
 * Three distinct outcomes are possible:
 *
 *   Successful update:
 *     success    = true
 *     notFound   = false
 *     resident   = updated Resident
 *     errors     = empty
 *
 *   Validation failure:
 *     success    = false
 *     notFound   = false
 *     resident   = std::nullopt
 *     errors     = failing field names
 *
 *   Resident not found:
 *     success    = false
 *     notFound   = true
 *     resident   = std::nullopt
 *     errors     = empty
 */
struct ResidentUpdateResult
{
    bool success{false};
    bool notFound{false};

    std::optional<Resident> resident{std::nullopt};

    std::vector<std::string> errors;
};

} // namespace csms
