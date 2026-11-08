#pragma once

#include "../repositories/ServiceRequestRepository.h"
#include "ServiceRequestStatusResult.h"

#include <string>

namespace csms {

/**
 * Manages the lifecycle status of a persisted Service Request.
 *
 * Responsibilities:
 * - Retrieve the existing Service Request by ID.
 * - Validate that the requested target status is supported.
 * - Enforce the allowed transition rules.
 * - Persist a valid status change through ServiceRequestRepository.
 * - Return a ServiceRequestStatusResult describing the outcome.
 *
 * Supported statuses:
 *   Pending, In Progress, Completed, Cancelled
 *
 * Allowed transitions:
 *   Pending     -> In Progress
 *   Pending     -> Cancelled
 *   In Progress -> Completed
 *   In Progress -> Cancelled
 *
 * Terminal states (no further transitions):
 *   Completed, Cancelled
 *
 * Same-status requests are always rejected as invalid transitions.
 *
 * This class does not:
 * - Create new Service Requests.
 * - Modify Resident information.
 * - Check Resident Active status for existing requests.
 * - Implement a presentation layer.
 */
class ServiceRequestStatusService
{
public:
    explicit ServiceRequestStatusService(
        ServiceRequestRepository& serviceRequestRepository);

    /**
     * Attempts to transition an existing Service Request to the requested status.
     *
     * Steps:
     *   1. Retrieve the Service Request by requestId.
     *   2. Return notFound if it does not exist.
     *   3. Validate that targetStatus is a supported status value.
     *   4. Determine whether the transition is allowed.
     *   5. If invalid, return invalidTransition without modifying persistence.
     *   6. Persist the updated status.
     *   7. Return the updated Service Request.
     */
    ServiceRequestStatusResult manageStatus(
        int requestId,
        const std::string& targetStatus);

private:
    ServiceRequestRepository& serviceRequestRepository_;

    // Returns true if the status string is one of the four supported values.
    bool isSupportedStatus(const std::string& status) const;

    // Returns true if the transition from currentStatus to targetStatus
    // is permitted by the T10 workflow rules.
    bool isAllowedTransition(
        const std::string& currentStatus,
        const std::string& targetStatus) const;
};

} // namespace csms
