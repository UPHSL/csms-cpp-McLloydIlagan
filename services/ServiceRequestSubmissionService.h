#pragma once

#include "../models/ServiceRequestValidator.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "ServiceRequestSubmissionResult.h"

namespace csms {

/**
 * Coordinates Service Request validation, Resident eligibility
 * verification, and persistence into a single submission operation.
 *
 * Responsibilities:
 * - Validate the ServiceRequest's intrinsic information.
 * - Verify the referenced Resident exists using ResidentRepository.
 * - Verify the Resident is Active.
 * - Persist a valid request through ServiceRequestRepository.
 * - Return a ServiceRequestSubmissionResult describing the outcome.
 *
 * This class does not:
 * - Own validation rules (delegated to ServiceRequestValidator).
 * - Own persistence logic (delegated to ServiceRequestRepository).
 * - Generate Service Request IDs (SQLite generates them).
 * - Modify the Resident.
 * - Implement status transitions.
 */
class ServiceRequestSubmissionService
{
public:
    ServiceRequestSubmissionService(
        const ServiceRequestValidator& validator,
        ResidentRepository& residentRepository,
        ServiceRequestRepository& serviceRequestRepository
    );

    ServiceRequestSubmissionResult submitRequest(
        const ServiceRequest& request
    );

private:
    const ServiceRequestValidator& validator_;
    ResidentRepository&            residentRepository_;
    ServiceRequestRepository&      serviceRequestRepository_;
};

} // namespace csms
