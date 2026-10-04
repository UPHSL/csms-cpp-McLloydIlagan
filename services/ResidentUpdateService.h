#pragma once

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"
#include "ResidentUpdateResult.h"

namespace csms {

/**
 * Coordinates the update of an existing persisted Resident.
 *
 * Responsibilities:
 * - Retrieve the existing Resident by id.
 * - Return a not-found result when the id does not exist.
 * - Preserve the existing Resident id and status.
 * - Apply the proposed editable fields to an update candidate.
 * - Validate the candidate using the T02 ResidentValidator.
 * - Return a validation-failure result when validation fails.
 * - Persist the valid update through ResidentRepository.
 * - Return the successfully updated Resident.
 *
 * This class does not own validation rules or SQL.
 * It does not change Resident status (that belongs to T07).
 * It does not create new Residents.
 */
class ResidentUpdateService
{
public:
    ResidentUpdateService(
        const ResidentValidator& validator,
        ResidentRepository& repository
    );

    /**
     * Updates the permitted information of the Resident identified by
     * residentId.
     *
     * The proposed object supplies new values for:
     *   firstName, lastName, address, contactNumber, email
     *
     * The existing persisted status is preserved automatically.
     * The id in the proposed object is ignored — residentId is used.
     *
     * Returns a ResidentUpdateResult describing the outcome.
     */
    ResidentUpdateResult updateResident(
        int residentId,
        const Resident& proposed
    );

private:
    const ResidentValidator& validator_;
    ResidentRepository& repository_;
};

} // namespace csms
