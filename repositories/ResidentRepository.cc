#include "ResidentRepository.h"

#include <stdexcept>
#include <string>

namespace csms {

ResidentRepository::ResidentRepository(Database& database)
    : database_(database)
{
}

Resident ResidentRepository::save(const Resident& resident)
{
    // Prepared INSERT statement.
    // Using '?' placeholders — values are bound separately, so no
    // Resident data is ever concatenated directly into the SQL string.
    // The id column is intentionally omitted so SQLite generates it.
    const char* sql =
        "INSERT INTO residents "
        "(first_name, last_name, address, contact_number, email, status) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    const int prepareResult = sqlite3_prepare_v2(
        database_.handle(),
        sql,
        -1,        // read the full null-terminated string
        &stmt,
        nullptr
    );

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            std::string("Failed to prepare INSERT: ") +
            sqlite3_errmsg(database_.handle())
        );
    }

    // Bind each Resident field to the numbered placeholder.
    // sqlite3_bind_text parameters: stmt, index (1-based), value, length, destructor.
    // SQLITE_TRANSIENT tells SQLite to copy the string immediately,
    // so it is safe even after the std::string goes out of scope.
    sqlite3_bind_text(stmt, 1, resident.getFirstName().c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, resident.getLastName().c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, resident.getAddress().c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, resident.getContactNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, resident.getEmail().c_str(),         -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, resident.getStatus().c_str(),        -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(stmt);

    if (stepResult != SQLITE_DONE)
    {
        const std::string message =
            std::string("INSERT failed: ") + sqlite3_errmsg(database_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error(message);
    }

    // Retrieve the identifier SQLite assigned to the new row.
    // sqlite3_last_insert_rowid returns the rowid of the most recent
    // successful INSERT on this connection.
    const int assignedId = static_cast<int>(
        sqlite3_last_insert_rowid(database_.handle())
    );

    // Always finalize the prepared statement to release SQLite resources.
    sqlite3_finalize(stmt);

    // Return a new Resident that carries the database-generated identifier
    // along with the original field values.
    return Resident(
        assignedId,
        resident.getFirstName(),
        resident.getLastName(),
        resident.getAddress(),
        resident.getContactNumber(),
        resident.getEmail(),
        resident.getStatus()
    );
}

std::optional<Resident> ResidentRepository::findById(int residentId)
{
    const char* sql =
        "SELECT id, first_name, last_name, address, "
        "       contact_number, email, status "
        "FROM residents "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    const int prepareResult = sqlite3_prepare_v2(
        database_.handle(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            std::string("Failed to prepare SELECT: ") +
            sqlite3_errmsg(database_.handle())
        );
    }

    // Bind the requested identifier to the single '?' placeholder.
    sqlite3_bind_int(stmt, 1, residentId);

    const int stepResult = sqlite3_step(stmt);

    if (stepResult == SQLITE_ROW)
    {
        // A row was found — read each column and build a Resident.
        // sqlite3_column_text returns a const unsigned char*, which is
        // safely converted to std::string via the reinterpret_cast.

        const int id = sqlite3_column_int(stmt, 0);

        const std::string firstName = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 1));

        const std::string lastName = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 2));

        const std::string address = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 3));

        const std::string contactNumber = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 4));

        const std::string email = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 5));

        const std::string status = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 6));

        sqlite3_finalize(stmt);

        return Resident(id, firstName, lastName, address, contactNumber, email, status);
    }

    // No row matched — finalize and return the not-found representation.
    sqlite3_finalize(stmt);
    return std::nullopt;
}

} // namespace csms
