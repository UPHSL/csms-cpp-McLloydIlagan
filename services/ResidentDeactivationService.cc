#include "ResidentDeactivationService.h"

namespace csms {

ResidentDeactivationService::ResidentDeactivationService(
    ResidentRepository& repository)
    : repository_(repository)
{
}

ResidentDeactivationResult ResidentDeactivationService::deactivateResident(
    int residentId)
{
    // Step 1 — Retrieve the existing Resident.
    const std::optional<Resident> existing =
        repository_.findById(residentId);

    if (!existing.has_value())
    {
        return ResidentDeactivationResult{
            false,   // success
            false,   // alreadyInactive
            true,    // notFound
            std::nullopt
        };
    }

    // Step 2 — Check the current status.
    // If already Inactive, return the safe repeated-operation result.
    // This is idempotent — the Resident stays Inactive and no UPDATE
    // is needed.
    if (existing->getStatus() == "Inactive")
    {
        return ResidentDeactivationResult{
            true,    // success
            true,    // alreadyInactive
            false,   // notFound
            existing // return the current persisted Resident
        };
    }

    // Step 3 — Persist the status change.
    // repository_.deactivateById() writes status = 'Inactive' and
    // returns the updated Resident. Personal/contact info is untouched.
    const Resident deactivated = repository_.deactivateById(residentId);

    return ResidentDeactivationResult{
        true,        // success
        false,       // alreadyInactive
        false,       // notFound
        deactivated
    };
}

} // namespace csms
