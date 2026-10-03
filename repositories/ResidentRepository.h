#pragma once

#include <optional>
#include <string>
#include <vector>

#include "database/Database.h"
#include "models/Resident.h"

namespace csms {

/**
 * Persists and retrieves Resident records using SQLite.
 *
 * Responsibilities:
 * - Store a valid Resident in the database.
 * - Assign the SQLite-generated identifier to the stored Resident.
 * - Retrieve a Resident by its identifier.
 * - Return std::nullopt when no Resident exists for a given identifier.
 * - Return all persisted Residents in deterministic order.
 * - Search Residents by partial, case-insensitive first or last name.
 *
 * This class does not validate Residents. Callers must supply
 * Residents that already satisfy the T02 validation rules.
 */
class ResidentRepository
{
public:
    // Constructs the repository with an open Database connection.
    explicit ResidentRepository(Database& database);

    // Non-copyable — the repository holds a reference to a Database.
    ResidentRepository(const ResidentRepository&) = delete;
    ResidentRepository& operator=(const ResidentRepository&) = delete;

    /**
     * Persists a Resident to the database.
     *
     * The supplied Resident must already satisfy T02 validation.
     * SQLite generates the identifier; the caller must NOT provide one.
     *
     * Returns a new Resident object that contains the database-generated id
     * along with the original field values.
     *
     * Throws std::runtime_error if the INSERT operation fails.
     */
    Resident save(const Resident& resident);

    /**
     * Retrieves a Resident by its database-generated identifier.
     *
     * Returns std::optional<Resident> containing the Resident if found.
     * Returns std::nullopt if no Resident exists with the given id.
     */
    std::optional<Resident> findById(int residentId);

    /**
     * Returns all persisted Residents ordered by:
     *   last name ascending (case-insensitive)
     *   then first name ascending (case-insensitive)
     *   then id ascending
     *
     * Returns an empty vector when no Residents exist.
     */
    std::vector<Resident> findAll();

    /**
     * Returns Residents whose first name or last name contains the
     * given search term (case-insensitive partial match).
     *
     * The search is performed entirely at the database level using
     * a LIKE query — no in-memory filtering is performed here.
     *
     * Results follow the same ordering as findAll().
     * Each matching Resident appears only once even if both names match.
     * Returns an empty vector when nothing matches.
     *
     * The searchTerm must already be trimmed by the caller.
     */
    std::vector<Resident> searchByName(const std::string& searchTerm);

private:
    Database& database_;

    // Reads all rows from the current statement and maps them into
    // Resident objects. Finalizes the statement before returning.
    std::vector<Resident> collectRows(sqlite3_stmt* stmt);
};

} // namespace csms
