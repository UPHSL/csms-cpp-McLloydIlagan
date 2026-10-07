#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/Resident.h"

namespace csms {

/**
 * Represents the outcome of one Resident registration attempt.
 *
 * Successful registration:
 *   success  = true
 *   resident = the persisted Resident (with database-generated id)
 *   errors   = empty
 *
 * Failed registration (validation errors):
 *   success  = false
 *   resident = std::nullopt
 *   errors   = list of field names that failed validation
 */
struct ResidentRegistrationResult
{
    bool success{false};

    std::optional<Resident> resident{
        std::nullopt
    };

    std::vector<std::string> errors;
};

} // namespace csms
