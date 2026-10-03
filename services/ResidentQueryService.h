#pragma once

#include <string>
#include <vector>

#include "../models/Resident.h"
#include "../repositories/ResidentRepository.h"

namespace csms {

/**
 * Coordinates Resident listing and name-search operations.
 *
 * Responsibilities:
 * - Return all persisted Residents in deterministic order.
 * - Accept a search term, trim it, and decide whether to list all
 *   Residents (blank search) or perform a partial name search.
 * - Delegate actual database queries to ResidentRepository.
 *
 * This class does not own persistence logic or SQL.
 * It does not filter Residents in memory — all filtering is done
 * by the repository at the database level.
 */
class ResidentQueryService
{
public:
    explicit ResidentQueryService(ResidentRepository& repository);

    /**
     * Returns all persisted Residents ordered by:
     *   last name ascending, first name ascending, id ascending.
     *
     * Returns an empty vector when no Residents exist.
     */
    std::vector<Resident> listResidents();

    /**
     * Searches Residents by partial, case-insensitive first or last name.
     *
     * The search term is trimmed of leading and trailing whitespace
     * before use. A blank term (empty after trimming) behaves the same
     * as listResidents() and returns all Residents.
     *
     * Returns matching Residents in the same order as listResidents().
     * Returns an empty vector when nothing matches.
     */
    std::vector<Resident> searchResidents(const std::string& searchTerm);

private:
    ResidentRepository& repository_;

    // Removes leading and trailing whitespace from a string.
    std::string trim(const std::string& value) const;
};

} // namespace csms
