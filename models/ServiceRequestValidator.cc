#include "ServiceRequestValidator.h"

#include <algorithm>
#include <cctype>

namespace csms {

std::vector<std::string> ServiceRequestValidator::validate(
    const ServiceRequest& request) const
{
    std::vector<std::string> errors;

    // Rule 1 — id must be unassigned before submission.
    // A request that already has a persisted id is not a new submission.
    if (request.getId().has_value())
    {
        errors.push_back("id");
    }

    // Rule 2 — residentId must be a positive integer.
    if (request.getResidentId() <= 0)
    {
        errors.push_back("residentId");
    }

    // Rule 3 — serviceType is required and cannot be whitespace-only.
    if (isBlank(request.getServiceType()))
    {
        errors.push_back("serviceType");
    }

    // Rule 4 — description is required and cannot be whitespace-only.
    if (isBlank(request.getDescription()))
    {
        errors.push_back("description");
    }

    // Rule 5 — dateRequested is required and cannot be whitespace-only.
    if (isBlank(request.getDateRequested()))
    {
        errors.push_back("dateRequested");
    }

    // Rule 6 — A new submission must begin as Pending.
    if (request.getStatus() != "Pending")
    {
        errors.push_back("status");
    }

    return errors;
}

bool ServiceRequestValidator::isValid(
    const ServiceRequest& request) const
{
    return validate(request).empty();
}

bool ServiceRequestValidator::isBlank(const std::string& value) const
{
    if (value.empty())
    {
        return true;
    }

    return std::all_of(
        value.begin(),
        value.end(),
        [](unsigned char c) { return std::isspace(c) != 0; }
    );
}

} // namespace csms
