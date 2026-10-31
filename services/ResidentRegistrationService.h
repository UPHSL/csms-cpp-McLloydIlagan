#pragma once

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"
#include "ResidentRegistrationResult.h"

namespace csms {

/**
 * Coordinates Resident validation and persistence into a single
 * registration operation.
 *
 * Responsibilities:
 * - Validate the supplied Resident using ResidentValidator.
 * - If validation passes, persist the Resident using ResidentRepository.
 * - Return a ResidentRegistrationResult describing the outcome.
 *
 * This class does not own validation rules or persistence logic.
 * Both responsibilities are delegated to the existing T02 and T03
 * components supplied through the constructor.
 */
class ResidentRegistrationService
{
public:
    ResidentRegistrationService(
        const ResidentValidator& validator,
        ResidentRepository& repository
    );

    ResidentRegistrationResult registerResident(
        const Resident& resident
    );

private:
    const ResidentValidator& validator_;
    ResidentRepository& repository_;
};

} // namespace csms
