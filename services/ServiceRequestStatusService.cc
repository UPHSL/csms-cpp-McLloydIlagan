#include "ServiceRequestStatusService.h"

namespace csms {

ServiceRequestStatusService::ServiceRequestStatusService(
    ServiceRequestRepository& serviceRequestRepository)
    : serviceRequestRepository_(serviceRequestRepository)
{
}

ServiceRequestStatusResult ServiceRequestStatusService::manageStatus(
    int requestId,
    const std::string& targetStatus)
{
    // Step 1 — Retrieve the existing Service Request.
    // T10 only works with already-persisted requests.
    const std::optional<ServiceRequest> existing =
        serviceRequestRepository_.findById(requestId);

    // Step 2 — Handle missing Service Request.
    if (!existing.has_value())
    {
        return ServiceRequestStatusResult{
            false,  // success
            true,   // notFound
            false,  // unsupportedStatus
            false,  // invalidTransition
            std::nullopt
        };
    }

    // Step 3 — Validate that the requested target status is supported.
    if (!isSupportedStatus(targetStatus))
    {
        return ServiceRequestStatusResult{
            false,  // success
            false,  // notFound
            true,   // unsupportedStatus
            false,  // invalidTransition
            std::nullopt
        };
    }

    // Step 4 — Determine whether the transition is allowed.
    // This includes same-status rejection and terminal-state protection.
    const std::string& currentStatus = existing->getStatus();

    if (!isAllowedTransition(currentStatus, targetStatus))
    {
        return ServiceRequestStatusResult{
            false,  // success
            false,  // notFound
            false,  // unsupportedStatus
            true,   // invalidTransition
            std::nullopt
        };
    }

    // Step 5 — Persist the valid status change.
    // Only the status column changes; all other fields remain intact.
    const ServiceRequest updated =
        serviceRequestRepository_.updateStatus(requestId, targetStatus);

    return ServiceRequestStatusResult{
        true,   // success
        false,  // notFound
        false,  // unsupportedStatus
        false,  // invalidTransition
        updated
    };
}

bool ServiceRequestStatusService::isSupportedStatus(
    const std::string& status) const
{
    return status == "Pending"     ||
           status == "In Progress" ||
           status == "Completed"   ||
           status == "Cancelled";
}

bool ServiceRequestStatusService::isAllowedTransition(
    const std::string& currentStatus,
    const std::string& targetStatus) const
{
    // Same-status requests are never valid transitions.
    if (currentStatus == targetStatus)
    {
        return false;
    }

    // Pending may move to In Progress or Cancelled.
    if (currentStatus == "Pending")
    {
        return targetStatus == "In Progress" ||
               targetStatus == "Cancelled";
    }

    // In Progress may move to Completed or Cancelled.
    if (currentStatus == "In Progress")
    {
        return targetStatus == "Completed" ||
               targetStatus == "Cancelled";
    }

    // Completed and Cancelled are terminal — no further transitions.
    // Any other combination (e.g. unknown current status) is also rejected.
    return false;
}

} // namespace csms
