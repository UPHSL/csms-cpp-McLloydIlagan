#pragma once

#include <string>
#include <vector>

#include "Resident.h"

namespace csms {

/**
 * Validates Resident information against the T02 validation rules.
 *
 * Responsibilities:
 * - Determine whether a Resident's information is valid or invalid.
 * - Return a list of field names that failed validation.
 *
 * This class does not persist, register, or modify Residents.
 */
class ResidentValidator
{
public:
    // Returns a list of field names that failed validation.
    // An empty vector means the Resident is valid.
    std::vector<std::string> validate(const Resident& resident) const;

    // Returns true if the Resident passes all validation rules.
    bool isValid(const Resident& resident) const;

private:
    bool isBlank(const std::string& value) const;
    bool isValidContactNumber(const std::string& value) const;
    bool isValidEmail(const std::string& value) const;
    bool isSupportedStatus(const std::string& value) const;
};

} // namespace csms
