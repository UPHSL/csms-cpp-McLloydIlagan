#pragma once

#include <optional>
#include <string>

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

private:
    Database& database_;
};

} // namespace csms
