#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/ServiceRequest.h"
#include "../repositories/ServiceRequestRepository.h"

namespace csms {

/**
 * Coordinates ServiceRequest listing, viewing, and search operations.
 *
 * Responsibilities:
 * - Return all persisted ServiceRequests in deterministic order.
 * - Return one ServiceRequest by id (delegates to repository findById).
 * - Accept a search term and optional status filter; trim and normalize
 *   inputs; decide whether to list all or perform a filtered search.
 * - Delegate all database queries to ServiceRequestRepository.
 *
 * This class does not own persistence logic or SQL.
 * All filtering is done by the repository at the database level.
 */
class ServiceRequestQueryService
{
public:
    explicit ServiceRequestQueryService(ServiceRequestRepository& repository);

    /**
     * Returns all persisted ServiceRequests ordered by:
     *   date_requested DESC, then id DESC.
     *
     * Returns an empty vector when no ServiceRequests exist.
     */
    std::vector<ServiceRequest> listServiceRequests();

    /**
     * Returns the ServiceRequest with the given id, or std::nullopt if
     * no such record exists.
     *
     * Delegates directly to ServiceRequestRepository::findById.
     * Does not create, insert, or modify any record.
     */
    std::optional<ServiceRequest> viewServiceRequest(int serviceRequestId);

    /**
     * Searches ServiceRequests by partial text and/or status filter.
     *
     * searchTerm is trimmed of leading/trailing whitespace before use.
     * A blank searchTerm (empty after trimming) and no statusFilter
     * behaves the same as listServiceRequests().
     *
     * statusFilter must be one of: "Pending", "In Progress",
     * "Completed", "Cancelled", or "" (empty = no status filter).
     * Any other non-empty value throws std::invalid_argument.
     *
     * The text search covers serviceType OR description (case-insensitive
     * partial match). All filtering occurs at the database level.
     *
     * Returns an empty vector when nothing matches.
     */
    std::vector<ServiceRequest> searchServiceRequests(
        const std::string& searchTerm,
        const std::string& statusFilter = "");

private:
    ServiceRequestRepository& repository_;

    // Removes leading and trailing whitespace. Identical logic to
    // ResidentQueryService::trim.
    std::string trim(const std::string& value) const;

    // Returns true when statusFilter is a recognised lifecycle status.
    bool isSupportedStatus(const std::string& status) const;
};

} // namespace csms
