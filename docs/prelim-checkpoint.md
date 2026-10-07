# Preliminary Examination Checkpoint — T03

## Developer Information

Name: Lloyd Ilagan
GitHub Username: McLloydIlagan
Primary Technology Stack: C++ with Drogon
T03 Branch: feature/t03-resident-persistence

---

## My T03 Implementation

For T03, I stored Resident data in a file-backed SQLite database — meaning the data actually lives on disk and survives between program runs, not just while the program is running in memory. I split the persistence work into two components: a `Database` class that handles opening and closing the SQLite connection, and a `ResidentRepository` that does the actual saving and retrieving.

When `save()` is called, the repository prepares an INSERT statement, binds each field from the Resident object to its corresponding placeholder, runs the statement, and then retrieves the id that SQLite automatically assigned to the new row using `sqlite3_last_insert_rowid`. It then constructs and returns a new Resident that includes that generated id alongside all the original data.

When `findById()` is called, the repository runs a SELECT query filtered by the given id. If SQLite finds a matching row, it reads each column and builds a Resident object from them, returning it inside a `std::optional`. If there's no row with that id, it returns `std::nullopt` — which is the clean C++ way of saying "nothing was found" without crashing or returning garbage.

---

## My Persistence Design Decision

I decided to put the SQLite connection and schema setup in a separate `Database` class instead of mixing it into `ResidentRepository`. My thinking was that the repository should only need to worry about storing and fetching Residents — not about opening files or running CREATE TABLE statements. Keeping those concerns separate made both classes easier to read and reason about.

The alternative I considered was just passing a raw `sqlite3*` pointer around, but that would have meant whoever created the pointer was also responsible for closing it, which gets messy. By wrapping it in `Database` with a destructor that calls `sqlite3_close`, I can rely on C++ to clean it up automatically when the object goes out of scope — this also made Test 7 (the two-repository test) work correctly, because the first connection closes cleanly before the second one opens the same file.

---

## My Database Resource Management

The connection is opened inside the `Database` constructor via `sqlite3_open`. If that call fails, the constructor closes the partially-created handle and throws an exception right away, so a broken connection is never handed to the rest of the program.

Closing the connection happens in the `Database` destructor via `sqlite3_close`. This runs automatically when the `Database` object goes out of scope, so I don't have to remember to call it manually.

For prepared statements, every `sqlite3_stmt*` in both `save()` and `findById()` is cleaned up with `sqlite3_finalize` before the function returns — whether it succeeds or fails. In `save()`, if `sqlite3_step` returns an error, I finalize the statement before throwing so nothing leaks. In `findById()`, I finalize on both the found and not-found paths. If `sqlite3_prepare_v2` itself fails, there's no valid statement to finalize, so I skip that and throw immediately.

---

## My Resident Table Design

Primary key design: `id INTEGER PRIMARY KEY AUTOINCREMENT` — this tells SQLite to generate a unique integer for every new row automatically. I used `AUTOINCREMENT` specifically to prevent SQLite from ever reusing an id that belonged to a deleted row.

Contact number column type: `TEXT NOT NULL` — phone numbers like `09171234567` need to stay as text. If I used `INTEGER`, SQLite would silently strip the leading zero and store `9171234567` instead, which would be wrong. Using `TEXT` keeps the value exactly as entered.

Status column type: `TEXT NOT NULL DEFAULT 'Active'` — the status field in the Resident model is a `std::string`, so storing it as `TEXT` is the most direct mapping. The default of `'Active'` matches the T01 domain model behavior.

All the other columns are `NOT NULL` because a Resident that's missing a name, address, contact number, email, or status wouldn't pass T02 validation anyway — so there's no valid scenario where those values would be empty when reaching the persistence layer.

I also used `CREATE TABLE IF NOT EXISTS` so the schema initialization can run multiple times without failing or deleting existing data.

---

## Files I Changed

File: `database/Database.h`
Purpose: Declares the `Database` class, which wraps a `sqlite3*` and manages its lifetime. It also exposes a `handle()` method so `ResidentRepository` can use the connection without owning it.

File: `database/Database.cc`
Purpose: Implements `Database`. The constructor opens the database file and runs the schema SQL. The destructor closes the connection. Connection errors are thrown immediately so the rest of the program never receives a broken handle.

File: `repositories/ResidentRepository.h`
Purpose: Declares `ResidentRepository` with the two public methods the exam requires: `save(const Resident&)` and `findById(int)`.

File: `repositories/ResidentRepository.cc`
Purpose: Implements the actual persistence logic. `save()` uses a prepared INSERT, binds all six fields, steps the statement, reads back the generated id, and returns a new Resident with that id. `findById()` uses a prepared SELECT, binds the id, and either maps the returned row into a Resident or returns `std::nullopt`.

File: `test/test_main.cc`
Purpose: Added the T03 persistence tests as plain helper functions called from a single `DROGON_TEST(ResidentPersistenceTest)` wrapper, following the same pattern as the T02 tests. All eight scenarios are covered. T01 and T02 tests are untouched.

File: `CMakeLists.txt`
Purpose: Added `database/Database.cc` and `repositories/ResidentRepository.cc` to the build, added `find_package(SQLite3 REQUIRED)`, linked against `SQLite3_lib`, and added `/FS` for MSVC so parallel builds don't conflict over the PDB file.

File: `test/CMakeLists.txt`
Purpose: Added the same two source files and the `SQLite3_lib` link to the test target so the test binary can compile and run the persistence code.

File: `.gitignore`
Purpose: Added `*.db`, `*.sqlite`, and `*.sqlite3` so the SQLite database file generated at runtime never accidentally gets committed.

---

## SQL I Can Explain

```sql
INSERT INTO residents
    (first_name, last_name, address, contact_number, email, status)
VALUES (?, ?, ?, ?, ?, ?);
```

This inserts one new row into the residents table. I intentionally left out the `id` column so SQLite assigns it automatically — I never want the application code deciding what the id should be.

Each `?` is a positional placeholder that gets filled in at runtime:
- placeholder 1 → `resident.getFirstName()`
- placeholder 2 → `resident.getLastName()`
- placeholder 3 → `resident.getAddress()`
- placeholder 4 → `resident.getContactNumber()`
- placeholder 5 → `resident.getEmail()`
- placeholder 6 → `resident.getStatus()`

Using placeholders instead of building the SQL string by hand means the Resident data is always treated as data, never as SQL syntax — so special characters or single quotes in a name won't break anything or cause an injection.

This statement is used in `ResidentRepository::save()`.

---

## My Resident Mapping

After `findById()` calls `sqlite3_step` and gets `SQLITE_ROW` back, I read each column by its index number:

- column 0 (`id`) → `sqlite3_column_int(stmt, 0)` → `int id`
- column 1 (`first_name`) → `sqlite3_column_text(stmt, 1)` → `std::string firstName`
- column 2 (`last_name`) → `sqlite3_column_text(stmt, 2)` → `std::string lastName`
- column 3 (`address`) → `sqlite3_column_text(stmt, 3)` → `std::string address`
- column 4 (`contact_number`) → `sqlite3_column_text(stmt, 4)` → `std::string contactNumber`
- column 5 (`email`) → `sqlite3_column_text(stmt, 5)` → `std::string email`
- column 6 (`status`) → `sqlite3_column_text(stmt, 6)` → `std::string status`

`sqlite3_column_text` returns `const unsigned char*`, which I cast to `const char*` before passing it to `std::string`. The most obvious mismatch between the database and C++ sides is the naming convention — the database uses `first_name` (snake_case) while the Resident model uses `firstName` (camelCase). The same applies to `last_name` → `lastName` and `contact_number` → `contactNumber`. Once all the values are read, I pass them to the Resident constructor from T01 that takes an id along with all six fields, and wrap the result in `std::optional`.

---

## Problem I Encountered

Problem or error: After I wrote the T03 tests, the CTest run would hang forever starting at test 6. The first five tests (starter, T01, T02) passed normally, but the process never exited after starting the persistence tests and had to be force-killed.

Cause: I originally wrote each of the eight scenarios as a separate `DROGON_TEST`, with each one calling `std::filesystem::remove()` at the end to delete the temporary database file. The problem was that `DROGON_TEST` bodies run on the Drogon IO event loop thread — so calling `sqlite3_close` followed immediately by a filesystem delete on that thread was blocking the loop long enough that the `app().quit()` signal never got processed, and the process just sat there. On top of that, after I rewrote the test file, CTest was still referencing the old test names from a stale `CTestTestfile.cmake`, so it was trying to run tests that no longer existed in the binary.

How I resolved it: I restructured the T03 tests to match the T02 pattern — plain `assert()`-based helper functions all called from a single `DROGON_TEST(ResidentPersistenceTest)` wrapper. This way the test body stays fast and doesn't do any blocking work on the event loop. I also moved the temporary file cleanup to the beginning of each helper (inside `makeTempDbPath()`) rather than the end, so it runs before the database opens rather than after it closes. Then I re-ran the CMake configure step to regenerate `CTestTestfile.cmake` with the correct test names, and all 6 CTest entries passed at 100%.

---

## My Student-Designed Test

Test name: `testContactNumberLeadingZeroSurvivesPersistence`

What it verifies: That a Philippine mobile number like `09171234567` comes back exactly the same after going through `save()` and `findById()` — the first character must still be `'0'` and the total length must still be 11.

Why I chose this scenario: The information-preservation test already checks that the contact number string equals the original, but that check would still pass even if the column type caused data corruption in some other way. I wanted a test that would fail specifically if the `contact_number` column were ever accidentally changed from `TEXT` to `INTEGER`, because SQLite would silently convert `09171234567` to the number `9171234567`, dropping the leading zero and shortening the string. That's a quiet data corruption that could go unnoticed for a long time — you'd only see the problem when someone tried to actually call or display the number and it came up one digit short.

---

## Tools and References Used

- **SQLite C/C++ API documentation** (https://www.sqlite.org/c3ref/intro.html) — for understanding how `sqlite3_open`, `sqlite3_prepare_v2`, `sqlite3_bind_text`, `sqlite3_step`, `sqlite3_finalize`, `sqlite3_last_insert_rowid`, and `sqlite3_column_text` work.
- **SQLite CREATE TABLE syntax** (https://www.sqlite.org/lang_createtable.html) — for `IF NOT EXISTS`, `INTEGER PRIMARY KEY AUTOINCREMENT`, and column constraints.
- **cppreference — std::optional** (https://en.cppreference.com/w/cpp/utility/optional) — to confirm the correct usage for `findById()`'s return type.
- **cppreference — std::filesystem** (https://en.cppreference.com/w/cpp/filesystem) — for `temp_directory_path()` and `remove()` in the test helper.
- **Drogon documentation** — to understand how `DROGON_TEST` registers and runs tests on the IO loop, which was essential for diagnosing the hang.
- **CMake documentation** — for `find_package`, `target_link_libraries`, and `add_compile_options`.
- **Kiro AI coding assistant** — helped with scaffolding the initial implementation files, diagnosing why the tests were hanging (the IO loop blocking issue), and figuring out the correct CMake target name for SQLite3. I reviewed all generated code and can explain every part of it.
