#pragma once

#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "database/Database.h"
#include "models/ServiceRequest.h"

namespace csms {

/**
 * Persists and retrieves ServiceRequest records using SQLite.
 *
 * Responsibilities:
 * - Store a valid ServiceRequest in the service_requests table.
 * - Assign the SQLite-generated identifier to the stored request.
 * - Retrieve a ServiceRequest by its identifier.
 * - Return std::nullopt when no ServiceRequest exists for a given id.
 *
 * This class does not validate ServiceRequests or verify Residents.
 * Callers must supply requests that have already passed T09 validation
 * and Resident eligibility checks.
 */
class ServiceRequestRepository
{
public:
    explicit ServiceRequestRepository(Database& database);

    ServiceRequestRepository(const ServiceRequestRepository&) = delete;
    ServiceRequestRepository& operator=(const ServiceRequestRepository&) = delete;

    /**
     * Persists a ServiceRequest to the database.
     *
     * The ServiceRequest must already be validated and its id must be
     * unassigned. SQLite generates the identifier.
     *
     * Returns a new ServiceRequest containing the database-generated id
     * and the original field values.
     *
     * Throws std::runtime_error if the INSERT fails.
     */
    ServiceRequest save(const ServiceRequest& request);

    /**
     * Retrieves a ServiceRequest by its database-generated identifier.
     *
     * Returns std::optional<ServiceRequest> if found.
     * Returns std::nullopt if no ServiceRequest exists with the given id.
     */
    std::optional<ServiceRequest> findById(int requestId);

    /**
     * Updates only the status column of an existing ServiceRequest.
     *
     * The caller is responsible for verifying that the ServiceRequest exists
     * and that the transition is valid before calling this method.
     *
     * Returns the full updated ServiceRequest retrieved from persistence
     * after the UPDATE so the caller receives the authoritative persisted state.
     *
     * Throws std::runtime_error if the UPDATE fails.
     */
    ServiceRequest updateStatus(int requestId, const std::string& newStatus);

    /**
     * Returns all persisted ServiceRequests ordered by:
     *   date_requested DESC, id DESC
     *
     * Returns an empty vector when no ServiceRequests exist.
     */
    std::vector<ServiceRequest> findAll();

    /**
     * Returns ServiceRequests matching the given optional criteria.
     *
     * searchTerm — partial match against LOWER(service_type) OR LOWER(description).
     *              Must already be lowercased by the caller. Pass "" to skip.
     * statusFilter — exact match against status column. Pass "" to skip.
     *
     * When both are empty, behaves identically to findAll().
     * All filtering is performed in SQLite; no in-memory filtering is done here.
     * DISTINCT prevents a record appearing twice when both columns match.
     * Results follow the same order as findAll().
     */
    std::vector<ServiceRequest> search(const std::string& searchTerm,
                                       const std::string& statusFilter);

private:
    Database& database_;

    // Reads all rows from the prepared statement and maps them into
    // ServiceRequest objects. Finalizes the statement before returning.
    std::vector<ServiceRequest> collectRows(sqlite3_stmt* stmt);
};

} // namespace csms
