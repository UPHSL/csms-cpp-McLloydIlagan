#pragma once

#include "../repositories/ResidentRepository.h"
#include "ResidentDeactivationResult.h"

namespace csms {

/**
 * Coordinates the soft deactivation of an existing Resident.
 *
 * Responsibilities:
 * - Retrieve the existing Resident by id.
 * - Return a not-found result when the id does not exist.
 * - Return a safe already-inactive result when the Resident is already Inactive.
 * - Ask the repository to change the status to Inactive for an Active Resident.
 * - Preserve all personal/contact information and the Resident id.
 * - Return the final persisted Resident state.
 *
 * This class does not:
 * - Revalidate personal information (T06 responsibility).
 * - Physically delete Residents.
 * - Implement reactivation or status toggling.
 * - Call the registration service.
 * - Create new Residents.
 */
class ResidentDeactivationService
{
public:
    explicit ResidentDeactivationService(ResidentRepository& repository);

    /**
     * Deactivates the Resident identified by residentId.
     *
     * Active  -> Inactive : success = true,  alreadyInactive = false
     * Inactive -> Inactive : success = true,  alreadyInactive = true
     * Not found            : success = false, notFound = true
     */
    ResidentDeactivationResult deactivateResident(int residentId);

private:
    ResidentRepository& repository_;
};

} // namespace csms
