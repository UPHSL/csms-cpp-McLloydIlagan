#include "Database.h"

namespace csms {

Database::Database(const std::string& dbPath)
{
    const int result = sqlite3_open(dbPath.c_str(), &db_);

    if (result != SQLITE_OK)
    {
        const std::string message = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Failed to open database: " + message);
    }

    initializeSchema();
}

Database::~Database()
{
    if (db_ != nullptr)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::initializeSchema()
{
    // CREATE TABLE IF NOT EXISTS ensures this is safe to call multiple times.
    // contact_number is TEXT so leading zeros (e.g. 09171234567) are preserved.
    // status is TEXT because the domain model allows open-ended string values.
    // id is INTEGER PRIMARY KEY, which is SQLite's alias for the rowid —
    //   SQLite auto-generates this value on every INSERT.
    const char* sql =
        "CREATE TABLE IF NOT EXISTS residents ("
        "    id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    first_name     TEXT NOT NULL,"
        "    last_name      TEXT NOT NULL,"
        "    address        TEXT NOT NULL,"
        "    contact_number TEXT NOT NULL,"
        "    email          TEXT NOT NULL,"
        "    status         TEXT NOT NULL DEFAULT 'Active'"
        ");";

    char* errorMessage = nullptr;
    const int result = sqlite3_exec(db_, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        const std::string message = errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to initialize schema: " + message);
    }
}

} // namespace csms
