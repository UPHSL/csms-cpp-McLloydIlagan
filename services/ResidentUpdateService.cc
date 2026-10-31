#include "ResidentUpdateService.h"

namespace csms {

ResidentUpdateService::ResidentUpdateService(
    const ResidentValidator& validator,
    ResidentRepository& repository
)
    : validator_(validator),
      repository_(repository)
{
}

ResidentUpdateResult ResidentUpdateService::updateResident(
    int residentId,
    const Resident& proposed
)
{
    // Step 1 — Retrieve the existing Resident.
    // If it does not exist, stop immediately and report not-found.
    const std::optional<Resident> existing =
        repository_.findById(residentId);

    if (!existing.has_value())
    {
        return ResidentUpdateResult{
            false,  // success
            true,   // notFound
            std::nullopt,
            {}
        };
    }

    // Step 2 — Build the update candidate.
    // Apply only the editable fields from the proposed object.
    // Preserve the existing id and status — they must not change.
    Resident candidate(
        existing->getId(),           // preserved id
        proposed.getFirstName(),
        proposed.getLastName(),
        proposed.getAddress(),
        proposed.getContactNumber(),
        proposed.getEmail(),
        existing->getStatus()        // preserved status
    );

    // Step 3 — Validate the candidate using the T02 validator.
    // Validation rules are not duplicated here.
    const std::vector<std::string> errors =
        validator_.validate(candidate);

    if (!errors.empty())
    {
        return ResidentUpdateResult{
            false,  // success
            false,  // notFound
            std::nullopt,
            errors
        };
    }

    // Step 4 — Persist the valid update.
    // repository_.update() targets only the existing row via its id.
    const Resident updated = repository_.update(candidate);

    return ResidentUpdateResult{
        true,   // success
        false,  // notFound
        updated,
        {}
    };
}

} // namespace csms
