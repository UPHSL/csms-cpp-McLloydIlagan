#pragma once

#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace csms {

/**
 * Manages a single SQLite database connection.
 *
 * Responsibilities:
 * - Open the SQLite database file.
 * - Initialize the schema (CREATE TABLE IF NOT EXISTS).
 * - Provide access to the raw sqlite3* handle for repository use.
 * - Close the connection when the object is destroyed (RAII).
 *
 * This class does not validate or persist Residents. It only handles
 * the low-level database connection and schema bootstrap.
 */
class Database
{
public:
    // Opens (or creates) the SQLite database at the given file path
    // and runs schema initialization.
    explicit Database(const std::string& dbPath);

    // Non-copyable — a database connection should not be shared via copy.
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Closes the SQLite connection on destruction.
    ~Database();

    // Returns the raw SQLite connection handle.
    // The handle is valid for the lifetime of this Database object.
    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_{nullptr};

    // Creates the residents table if it does not already exist.
    void initializeSchema();
};

} // namespace csms
