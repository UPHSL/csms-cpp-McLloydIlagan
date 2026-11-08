#include "ServiceRequestQueryService.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace csms {

ServiceRequestQueryService::ServiceRequestQueryService(
    ServiceRequestRepository& repository)
    : repository_(repository)
{
}

// ---------------------------------------------------------------------------
// listServiceRequests
// ---------------------------------------------------------------------------

std::vector<ServiceRequest> ServiceRequestQueryService::listServiceRequests()
{
    return repository_.findAll();
}

// ---------------------------------------------------------------------------
// viewServiceRequest
// ---------------------------------------------------------------------------

std::optional<ServiceRequest>
ServiceRequestQueryService::viewServiceRequest(int serviceRequestId)
{
    // Delegates directly to the existing T09 repository method.
    // Returns std::nullopt when the id does not exist — no record is created.
    return repository_.findById(serviceRequestId);
}

// ---------------------------------------------------------------------------
// searchServiceRequests
// ---------------------------------------------------------------------------

std::vector<ServiceRequest> ServiceRequestQueryService::searchServiceRequests(
    const std::string& searchTerm,
    const std::string& statusFilter)
{
    const std::string trimmedTerm = trim(searchTerm);

    // Validate statusFilter when it is non-empty.
    if (!statusFilter.empty() && !isSupportedStatus(statusFilter))
    {
        throw std::invalid_argument(
            "Unsupported status filter: " + statusFilter);
    }

    // Blank text + no status = list all.
    if (trimmedTerm.empty() && statusFilter.empty())
    {
        return repository_.findAll();
    }

    // Lowercase the trimmed term for case-insensitive LIKE matching.
    std::string lowerTerm = trimmedTerm;
    std::transform(lowerTerm.begin(), lowerTerm.end(), lowerTerm.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return repository_.search(lowerTerm, statusFilter);
}

// ---------------------------------------------------------------------------
// trim (private)
// ---------------------------------------------------------------------------

std::string ServiceRequestQueryService::trim(const std::string& value) const
{
    const auto start = std::find_if_not(
        value.begin(),
        value.end(),
        [](unsigned char c) { return std::isspace(c); });

    const auto end = std::find_if_not(
        value.rbegin(),
        value.rend(),
        [](unsigned char c) { return std::isspace(c); }).base();

    return (start < end) ? std::string(start, end) : std::string{};
}

// ---------------------------------------------------------------------------
// isSupportedStatus (private)
// ---------------------------------------------------------------------------

bool ServiceRequestQueryService::isSupportedStatus(
    const std::string& status) const
{
    return status == "Pending"     ||
           status == "In Progress" ||
           status == "Completed"   ||
           status == "Cancelled";
}

} // namespace csms
