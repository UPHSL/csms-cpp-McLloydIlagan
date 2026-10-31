#pragma once

#include <string>
#include <vector>

#include "ServiceRequest.h"

namespace csms {

/**
 * Validates the intrinsic information of a ServiceRequest.
 *
 * Responsibilities:
 * - Check that the Service Request ID is unassigned before submission.
 * - Check that residentId is a positive integer.
 * - Check that serviceType is present and not whitespace-only.
 * - Check that description is present and not whitespace-only.
 * - Check that dateRequested is present and not whitespace-only.
 * - Check that status is "Pending" for a new submission.
 *
 * This class does NOT:
 * - Perform Resident database lookups.
 * - Check whether the Resident exists or is Active.
 * - Validate the final allowlist of service types.
 *
 * Those cross-domain concerns belong to ServiceRequestSubmissionService.
 */
class ServiceRequestValidator
{
public:
    // Returns a list of field names that failed validation.
    // An empty vector means the ServiceRequest is valid for submission.
    std::vector<std::string> validate(
        const ServiceRequest& request) const;

    // Returns true when validate() produces no errors.
    bool isValid(const ServiceRequest& request) const;

private:
    bool isBlank(const std::string& value) const;
};

} // namespace csms
