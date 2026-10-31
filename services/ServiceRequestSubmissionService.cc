#include "ServiceRequestSubmissionService.h"

namespace csms {

ServiceRequestSubmissionService::ServiceRequestSubmissionService(
    const ServiceRequestValidator& validator,
    ResidentRepository& residentRepository,
    ServiceRequestRepository& serviceRequestRepository)
    : validator_(validator),
      residentRepository_(residentRepository),
      serviceRequestRepository_(serviceRequestRepository)
{
}

ServiceRequestSubmissionResult
ServiceRequestSubmissionService::submitRequest(
    const ServiceRequest& request)
{
    // Step 1 — Validate the ServiceRequest's intrinsic information.
    // Validation rules stay in ServiceRequestValidator, not here.
    const std::vector<std::string> errors =
        validator_.validate(request);

    if (!errors.empty())
    {
        return ServiceRequestSubmissionResult{
            false,   // success
            false,   // residentNotFound
            false,   // residentInactive
            std::nullopt,
            errors
        };
    }

    // Step 2 — Verify the referenced Resident exists.
    // Resident lookup uses the existing T03 ResidentRepository.
    const std::optional<Resident> resident =
        residentRepository_.findById(request.getResidentId());

    if (!resident.has_value())
    {
        return ServiceRequestSubmissionResult{
            false,   // success
            true,    // residentNotFound
            false,   // residentInactive
            std::nullopt,
            {}
        };
    }

    // Step 3 — Verify the Resident is Active.
    // Inactive Residents cannot submit new Service Requests (T07 rule).
    if (resident->getStatus() != "Active")
    {
        return ServiceRequestSubmissionResult{
            false,   // success
            false,   // residentNotFound
            true,    // residentInactive
            std::nullopt,
            {}
        };
    }

    // Step 4 — Persist the valid Service Request.
    // ID generation is handled by SQLite via ServiceRequestRepository.
    const ServiceRequest persisted =
        serviceRequestRepository_.save(request);

    return ServiceRequestSubmissionResult{
        true,    // success
        false,   // residentNotFound
        false,   // residentInactive
        persisted,
        {}
    };
}

} // namespace csms
