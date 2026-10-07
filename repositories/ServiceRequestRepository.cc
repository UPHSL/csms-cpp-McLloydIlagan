#include "ServiceRequestRepository.h"

#include <stdexcept>
#include <string>

namespace csms {

ServiceRequestRepository::ServiceRequestRepository(Database& database)
    : database_(database)
{
}

ServiceRequest ServiceRequestRepository::save(const ServiceRequest& request)
{
    // INSERT without id — SQLite generates it via AUTOINCREMENT.
    const char* sql =
        "INSERT INTO service_requests "
        "(resident_id, service_type, description, date_requested, status) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    const int prepareResult = sqlite3_prepare_v2(
        database_.handle(), sql, -1, &stmt, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            std::string("Failed to prepare service_requests INSERT: ") +
            sqlite3_errmsg(database_.handle()));
    }

    sqlite3_bind_int (stmt, 1, request.getResidentId());
    sqlite3_bind_text(stmt, 2, request.getServiceType().c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, request.getDescription().c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, request.getDateRequested().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, request.getStatus().c_str(),        -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(stmt);

    if (stepResult != SQLITE_DONE)
    {
        const std::string message =
            std::string("service_requests INSERT failed: ") +
            sqlite3_errmsg(database_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error(message);
    }

    const int assignedId = static_cast<int>(
        sqlite3_last_insert_rowid(database_.handle()));

    sqlite3_finalize(stmt);

    // Return a ServiceRequest that carries the database-generated id.
    ServiceRequest saved(
        assignedId,
        request.getResidentId(),
        request.getServiceType(),
        request.getDescription(),
        request.getDateRequested(),
        request.getStatus()
    );
    return saved;
}

std::optional<ServiceRequest> ServiceRequestRepository::findById(int requestId)
{
    const char* sql =
        "SELECT id, resident_id, service_type, description, "
        "       date_requested, status "
        "FROM service_requests "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    const int prepareResult = sqlite3_prepare_v2(
        database_.handle(), sql, -1, &stmt, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            std::string("Failed to prepare service_requests SELECT: ") +
            sqlite3_errmsg(database_.handle()));
    }

    sqlite3_bind_int(stmt, 1, requestId);

    const int stepResult = sqlite3_step(stmt);

    if (stepResult == SQLITE_ROW)
    {
        const int id = sqlite3_column_int(stmt, 0);
        const int residentId = sqlite3_column_int(stmt, 1);

        const std::string serviceType = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 2));
        const std::string description = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 3));
        const std::string dateRequested = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 4));
        const std::string status = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 5));

        sqlite3_finalize(stmt);

        return ServiceRequest(id, residentId, serviceType,
                              description, dateRequested, status);
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

ServiceRequest ServiceRequestRepository::updateStatus(
    int requestId,
    const std::string& newStatus)
{
    // UPDATE only the status column — all other fields remain unchanged.
    const char* sql =
        "UPDATE service_requests "
        "SET status = ? "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    const int prepareResult = sqlite3_prepare_v2(
        database_.handle(), sql, -1, &stmt, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            std::string("Failed to prepare service_requests UPDATE: ") +
            sqlite3_errmsg(database_.handle()));
    }

    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, requestId);

    const int stepResult = sqlite3_step(stmt);

    if (stepResult != SQLITE_DONE)
    {
        const std::string message =
            std::string("service_requests UPDATE failed: ") +
            sqlite3_errmsg(database_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error(message);
    }

    sqlite3_finalize(stmt);

    // Return the authoritative persisted state after the UPDATE.
    // findById is guaranteed to succeed because the caller already
    // verified the record exists before calling updateStatus.
    const std::optional<ServiceRequest> updated = findById(requestId);
    if (!updated.has_value())
    {
        throw std::runtime_error(
            "service_requests UPDATE succeeded but findById returned nullopt");
    }
    return updated.value();
}

} // namespace csms
