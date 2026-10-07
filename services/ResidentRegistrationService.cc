#include "ResidentRegistrationService.h"

namespace csms {

ResidentRegistrationService::ResidentRegistrationService(
    const ResidentValidator& validator,
    ResidentRepository& repository
)
    : validator_(validator),
      repository_(repository)
{
}

ResidentRegistrationResult
ResidentRegistrationService::registerResident(
    const Resident& resident
)
{
    // Ask the T02 validator whether the Resident is valid.
    // Validation rules are not repeated here — they belong to
    // ResidentValidator.
    const std::vector<std::string> errors =
        validator_.validate(resident);

    if (!errors.empty())
    {
        // Validation failed — do not persist.
        // Return the errors so the caller knows what went wrong.
        return ResidentRegistrationResult{
            false,
            std::nullopt,
            errors
        };
    }

    // Validation passed — delegate persistence to the T03 repository.
    // ID generation is handled by SQLite, not by this service.
    Resident persistedResident =
        repository_.save(resident);

    return ResidentRegistrationResult{
        true,
        persistedResident,
        {}
    };
}

} // namespace csms
