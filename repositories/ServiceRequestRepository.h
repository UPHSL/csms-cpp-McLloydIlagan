#pragma once

#include <optional>
#include <string>

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

private:
    Database& database_;
};

} // namespace csms
