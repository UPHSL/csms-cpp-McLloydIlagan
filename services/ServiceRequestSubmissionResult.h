#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/ServiceRequest.h"

namespace csms {

/**
 * Represents the outcome of one Service Request submission attempt.
 *
 * Four distinct outcomes are possible:
 *
 *   Successful submission:
 *     success          = true
 *     residentNotFound = false
 *     residentInactive = false
 *     serviceRequest   = persisted ServiceRequest (with generated id)
 *     errors           = empty
 *
 *   Validation failure:
 *     success          = false
 *     residentNotFound = false
 *     residentInactive = false
 *     serviceRequest   = std::nullopt
 *     errors           = failing field names
 *
 *   Resident not found:
 *     success          = false
 *     residentNotFound = true
 *     residentInactive = false
 *     serviceRequest   = std::nullopt
 *     errors           = empty
 *
 *   Resident inactive:
 *     success          = false
 *     residentNotFound = false
 *     residentInactive = true
 *     serviceRequest   = std::nullopt
 *     errors           = empty
 */
struct ServiceRequestSubmissionResult
{
    bool success{false};
    bool residentNotFound{false};
    bool residentInactive{false};

    std::optional<ServiceRequest> serviceRequest{std::nullopt};

    std::vector<std::string> errors;
};

} // namespace csms
