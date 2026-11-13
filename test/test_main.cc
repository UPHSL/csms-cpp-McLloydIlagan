#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>

#include "models/Resident.h"
#include "models/ResidentValidator.h"
#include "database/Database.h"
#include "repositories/ResidentRepository.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helper — validation
// ---------------------------------------------------------------------------

bool containsValidationError(
    const std::vector<std::string>& errors,
    const std::string& field)
{
    return std::find(errors.begin(), errors.end(), field) != errors.end();
}

// ---------------------------------------------------------------------------
// Helper — persistence
//
// Returns a temporary SQLite file path unique to the given label.
// Any leftover file from a previous failed run is removed first so
// every test execution starts with a clean database.
// ---------------------------------------------------------------------------

static std::string makeTempDbPath(const std::string& label)
{
    auto base = std::filesystem::temp_directory_path();
    base /= ("csms_test_" + label + ".db");
    std::filesystem::remove(base);     // ignore if missing
    return base.string();
}

// ---------------------------------------------------------------------------
// Starter test — must continue to pass
// ---------------------------------------------------------------------------

DROGON_TEST(BasicTest)
{
    // Existing placeholder test
}

// ---------------------------------------------------------------------------
// T01 Tests
// ---------------------------------------------------------------------------

// Test 1: Resident Creation
DROGON_TEST(ResidentCreationTest)
{
    csms::Resident resident(
        1,
        "Juan",
        "dela Cruz",
        "123 Mabini St, Manila",
        "09171234567",
        "juan.delacruz@email.com",
        "Active"
    );

    CHECK(resident.getId() == 1);
    CHECK(resident.getFirstName() == "Juan");
    CHECK(resident.getLastName() == "dela Cruz");
}

// Test 2: Resident Information Access
DROGON_TEST(ResidentInformationAccessTest)
{
    csms::Resident resident;

    resident.setId(2);
    resident.setFirstName("Maria");
    resident.setLastName("Santos");
    resident.setAddress("456 Rizal Ave, Quezon City");
    resident.setContactNumber("09281234567");
    resident.setEmail("maria.santos@email.com");
    resident.setStatus("Active");

    CHECK(resident.getId() == 2);
    CHECK(resident.getFirstName() == "Maria");
    CHECK(resident.getLastName() == "Santos");
    CHECK(resident.getAddress() == "456 Rizal Ave, Quezon City");
    CHECK(resident.getContactNumber() == "09281234567");
    CHECK(resident.getEmail() == "maria.santos@email.com");
    CHECK(resident.getStatus() == "Active");
}

// Test 3: Resident Status
DROGON_TEST(ResidentStatusTest)
{
    csms::Resident resident(
        3,
        "Pedro",
        "Reyes",
        "789 Bonifacio St, Pasig",
        "09391234567",
        "pedro.reyes@email.com",
        "Active"
    );

    CHECK(resident.getStatus() == "Active");
}

// ---------------------------------------------------------------------------
// T02 Validation Test Functions
// ---------------------------------------------------------------------------

void testValidResidentInformationPassesValidation()
{
    csms::Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    assert(validator.isValid(resident));
}

void testMissingFirstNameFailsValidation()
{
    csms::Resident resident(
        "",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "firstName"));
}

void testMissingLastNameFailsValidation()
{
    csms::Resident resident(
        "Juan",
        "",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "lastName"));
}

void testMissingAddressFailsValidation()
{
    csms::Resident resident(
        "Juan",
        "Dela Cruz",
        "",
        "09171234567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "address"));
}

void testWhitespaceOnlyRequiredInformationFailsValidation()
{
    csms::Resident resident(
        "   ",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "firstName"));
}

void testInvalidContactNumberFailsValidation()
{
    csms::Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "0917ABC4567",
        "juan@example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "contactNumber"));
}

void testInvalidEmailFailsValidation()
{
    csms::Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan.example.com"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "email"));
}

void testSupportedResidentStatusesPassValidation()
{
    csms::Resident activeResident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    csms::Resident inactiveResident(
        "Maria",
        "Santos",
        "Barangay Santo Tomas",
        "09181234567",
        "maria@example.com",
        "Inactive"
    );

    csms::ResidentValidator validator;

    assert(validator.isValid(activeResident));
    assert(validator.isValid(inactiveResident));
}

void testUnsupportedResidentStatusFailsValidation()
{
    csms::Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        "Unknown"
    );

    csms::ResidentValidator validator;
    const auto errors = validator.validate(resident);

    assert(!validator.isValid(resident));
    assert(containsValidationError(errors, "status"));
}

// ---------------------------------------------------------------------------
// T02 Drogon test wrapper — runs all T02 scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentValidationTest)
{
    testValidResidentInformationPassesValidation();
    testMissingFirstNameFailsValidation();
    testMissingLastNameFailsValidation();
    testMissingAddressFailsValidation();
    testWhitespaceOnlyRequiredInformationFailsValidation();
    testInvalidContactNumberFailsValidation();
    testInvalidEmailFailsValidation();
    testSupportedResidentStatusesPassValidation();
    testUnsupportedResidentStatusFailsValidation();
}

// ---------------------------------------------------------------------------
// T03 Persistence Test Helper Functions
//
// Each function uses assert() for the same pattern established in T02.
// All T03 helper functions are called from a single DROGON_TEST wrapper
// (ResidentPersistenceTest) so the Drogon event loop is not blocked by
// the SQLite I/O — the test body completes synchronously and quickly.
// ---------------------------------------------------------------------------

// Test 1: Persist a Resident
void testPersistResident()
{
    const std::string dbPath = makeTempDbPath("t03_persist");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    csms::Resident resident(
        "Ana",
        "Reyes",
        "12 Sampaguita St, Marikina",
        "09171234567",
        "ana.reyes@email.com"
    );

    // save() must complete without throwing and return the persisted Resident.
    csms::Resident saved = repo.save(resident);

    assert(saved.getFirstName() == "Ana");
    assert(saved.getLastName()  == "Reyes");
}

// Test 2: Resident Receives an Identifier
void testResidentReceivesIdentifier()
{
    const std::string dbPath = makeTempDbPath("t03_identifier");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    // id_ defaults to 0 — the T01 unassigned sentinel — before persistence.
    csms::Resident resident(
        "Ben",
        "Santos",
        "34 Mabini St, Pasig",
        "09281234567",
        "ben.santos@email.com"
    );
    assert(resident.getId() == 0);

    csms::Resident saved = repo.save(resident);

    // After persistence SQLite must have assigned a positive integer id.
    assert(saved.getId() > 0);
}

// Test 3: Retrieve Resident by Identifier
void testRetrieveResidentById()
{
    const std::string dbPath = makeTempDbPath("t03_retrieve");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    csms::Resident resident(
        "Clara",
        "Villanueva",
        "56 Rizal Ave, Quezon City",
        "09391234567",
        "clara.v@email.com"
    );

    const int assignedId = repo.save(resident).getId();

    std::optional<csms::Resident> found = repo.findById(assignedId);

    assert(found.has_value());
    assert(found->getId() == assignedId);
}

// Test 4: Resident Information Is Preserved
void testResidentInformationPreserved()
{
    const std::string dbPath = makeTempDbPath("t03_information");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    csms::Resident resident(
        "Diego",
        "Mercado",
        "78 Bonifacio St, Taguig",
        "09171112233",
        "diego.mercado@email.com",
        "Active"
    );

    const int id = repo.save(resident).getId();
    std::optional<csms::Resident> found = repo.findById(id);

    assert(found.has_value());
    assert(found->getFirstName()     == "Diego");
    assert(found->getLastName()      == "Mercado");
    assert(found->getAddress()       == "78 Bonifacio St, Taguig");
    // Leading zero must be preserved — stored as TEXT, not INTEGER.
    assert(found->getContactNumber() == "09171112233");
    assert(found->getEmail()         == "diego.mercado@email.com");
    assert(found->getStatus()        == "Active");
}

// Test 5: Active Status Is Preserved
void testActiveStatusPreserved()
{
    const std::string dbPath = makeTempDbPath("t03_status");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    // Default constructor parameter gives "Active" status.
    csms::Resident resident(
        "Elena",
        "Pascual",
        "90 Del Pilar St, Makati",
        "09181112233",
        "elena.pascual@email.com"
        // status defaults to "Active"
    );

    const int id = repo.save(resident).getId();
    std::optional<csms::Resident> found = repo.findById(id);

    assert(found.has_value());
    assert(found->getStatus() == "Active");
}

// Test 6: Missing Resident Is Handled
void testMissingResidentHandled()
{
    const std::string dbPath = makeTempDbPath("t03_missing");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    // 999999 is a deliberately nonexistent identifier.
    std::optional<csms::Resident> result = repo.findById(999999);

    // The repository must return nullopt — not crash or invent a Resident.
    assert(!result.has_value());
}

// Test 7: Persistence Is Not Limited to One Repository Object
void testPersistenceAcrossRepositoryInstances()
{
    const std::string dbPath = makeTempDbPath("t03_two_repos");

    int savedId = 0;

    // First repository instance: save a Resident, then close the connection.
    {
        csms::Database db1(dbPath);
        csms::ResidentRepository repo1(db1);

        csms::Resident resident(
            "Fernando",
            "Lim",
            "101 Luna St, Mandaluyong",
            "09291112233",
            "fernando.lim@email.com"
        );

        savedId = repo1.save(resident).getId();
        // db1 destructor calls sqlite3_close here — connection fully closed.
    }

    // Second repository instance: open the same file and retrieve the record.
    {
        csms::Database db2(dbPath);
        csms::ResidentRepository repo2(db2);

        std::optional<csms::Resident> found = repo2.findById(savedId);

        // The record must survive across repository instances because it is
        // stored in the SQLite file, not only in memory.
        assert(found.has_value());
        assert(found->getFirstName() == "Fernando");
        assert(found->getLastName()  == "Lim");
        assert(found->getId()        == savedId);
    }
}

// ---------------------------------------------------------------------------
// T03 Student-Designed Test
// Name: testContactNumberLeadingZeroSurvivesPersistence
//
// What it verifies:
//   A Philippine mobile number beginning with '0' (e.g. 09171234567)
//   is stored as TEXT and comes back byte-for-byte identical after a
//   full save-and-retrieve cycle.
//
// Why I chose this scenario:
//   Storing a phone number in a numeric column silently drops the leading
//   zero (09171234567 becomes 9171234567).  This defect is not caught by
//   the general information-preservation test unless the first character
//   is checked explicitly.  A dedicated test makes the TEXT-column
//   requirement visible and will fail immediately if the schema is
//   accidentally changed to INTEGER.
// ---------------------------------------------------------------------------

void testContactNumberLeadingZeroSurvivesPersistence()
{
    const std::string dbPath = makeTempDbPath("t03_leading_zero");
    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);

    const std::string originalNumber = "09171234567";

    csms::Resident resident(
        "Gloria",
        "Tan",
        "22 Katipunan Ave, Quezon City",
        originalNumber,
        "gloria.tan@email.com"
    );

    const int id = repo.save(resident).getId();
    std::optional<csms::Resident> found = repo.findById(id);

    assert(found.has_value());

    const std::string retrieved = found->getContactNumber();

    // The full 11-character string must be identical.
    assert(retrieved == originalNumber);

    // The leading character must be '0', not '9'.
    // An INTEGER column would strip it, making retrieved[0] == '9'.
    assert(!retrieved.empty());
    assert(retrieved[0] == '0');
    assert(retrieved.size() == 11u);
}

// ---------------------------------------------------------------------------
// T03 Drogon test wrapper — runs all T03 persistence scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentPersistenceTest)
{
    testPersistResident();
    testResidentReceivesIdentifier();
    testRetrieveResidentById();
    testResidentInformationPreserved();
    testActiveStatusPreserved();
    testMissingResidentHandled();
    testPersistenceAcrossRepositoryInstances();
    testContactNumberLeadingZeroSurvivesPersistence();
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    std::thread thr([&]() {
        // Suppress all Drogon output and disable plugins/listeners
        // so the test process exits cleanly after each CTest run.
        app().setLogLevel(trantor::Logger::kFatal);
        app().disableSigtermHandling();
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    f1.get();
    int status = test::run(argc, argv);

    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}

// ---------------------------------------------------------------------------
// T04 includes
// ---------------------------------------------------------------------------

#include "services/ResidentRegistrationService.h"
#include <sqlite3.h>

// ---------------------------------------------------------------------------
// T04 Helper — valid Resident for registration tests
// ---------------------------------------------------------------------------

static csms::Resident makeValidResidentForRegistration()
{
    return csms::Resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );
}

// ---------------------------------------------------------------------------
// T04 Helper — count rows in the residents table
//
// Used by Test 7 to prove that an invalid Resident was never persisted.
// Opens its own short-lived connection so it does not interfere with the
// Database instance held by the test.
// ---------------------------------------------------------------------------

static int countResidentsInDatabase(const std::string& databasePath)
{
    sqlite3* db = nullptr;
    sqlite3_open(databasePath.c_str(), &db);

    const char* sql = "SELECT COUNT(*) FROM residents";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_step(stmt);

    const int count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return count;
}

// ---------------------------------------------------------------------------
// T04 Test Helper Functions
//
// All eight T04 scenarios follow the same pattern as T03:
// plain assert()-based helpers called from one DROGON_TEST wrapper.
//
// Each helper creates its own isolated temporary SQLite file via
// makeTempDbPath() so tests cannot share state.
//
// T03 architecture note:
//   ResidentRepository takes Database& — not a file path directly.
//   We always construct: Database db(path); ResidentRepository repo(db);
//
// getId() returns int — 0 means unassigned (T01 sentinel).
// After persistence, getId() > 0.
// ---------------------------------------------------------------------------

// Test 1 — Register a valid Resident
void testRegisterValidResident()
{
    const std::string dbPath = makeTempDbPath("t04_register");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident resident = makeValidResidentForRegistration();

        const csms::ResidentRegistrationResult result =
            service.registerResident(resident);

        assert(result.success);
        assert(result.resident.has_value());
        assert(result.errors.empty());
    }
}

// Test 2 — Registered Resident receives a database-generated identifier
void testRegisteredResidentReceivesIdentifier()
{
    const std::string dbPath = makeTempDbPath("t04_identifier");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident resident = makeValidResidentForRegistration();

        // Before registration, id is 0 — the T01 unassigned sentinel.
        assert(resident.getId() == 0);

        const csms::ResidentRegistrationResult result =
            service.registerResident(resident);

        assert(result.success);
        assert(result.resident.has_value());

        // After registration, SQLite must have assigned a positive id.
        assert(result.resident->getId() > 0);
    }
}

// Test 3 — Registered Resident is actually persisted in SQLite
void testRegisteredResidentIsPersisted()
{
    const std::string dbPath = makeTempDbPath("t04_persisted");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident resident = makeValidResidentForRegistration();

        const csms::ResidentRegistrationResult result =
            service.registerResident(resident);

        assert(result.success);
        assert(result.resident.has_value());

        const int residentId = result.resident->getId();
        assert(residentId > 0);

        // Verify the record actually exists in SQLite.
        const std::optional<csms::Resident> stored =
            repo.findById(residentId);

        assert(stored.has_value());
    }
}

// Test 4 — Resident information is preserved through registration
void testRegisteredResidentInformationIsPreserved()
{
    const std::string dbPath = makeTempDbPath("t04_information");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident resident = makeValidResidentForRegistration();

        const csms::ResidentRegistrationResult result =
            service.registerResident(resident);

        assert(result.success);
        assert(result.resident.has_value());

        const int residentId = result.resident->getId();

        const std::optional<csms::Resident> stored =
            repo.findById(residentId);

        assert(stored.has_value());
        assert(stored->getFirstName()     == "Juan");
        assert(stored->getLastName()      == "Dela Cruz");
        assert(stored->getAddress()       == "Barangay Santo Tomas");
        // Leading zero must be preserved — stored as TEXT.
        assert(stored->getContactNumber() == "09171234567");
        assert(stored->getEmail()         == "juan@example.com");
        assert(stored->getStatus()        == "Active");
    }
}

// Test 5 — Default Active status is preserved through registration
void testRegisteredResidentPreservesDefaultActiveStatus()
{
    const std::string dbPath = makeTempDbPath("t04_status");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident resident = makeValidResidentForRegistration();

        // T01 default status — must not be set explicitly here.
        assert(resident.getStatus() == "Active");

        const csms::ResidentRegistrationResult result =
            service.registerResident(resident);

        assert(result.success);
        assert(result.resident.has_value());
        assert(result.resident->getStatus() == "Active");
    }
}

// Test 6 — Invalid Resident registration fails
void testInvalidResidentRegistrationFails()
{
    const std::string dbPath = makeTempDbPath("t04_invalid");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        // Empty first name — T02 validator must reject this.
        csms::Resident invalidResident(
            "",
            "Dela Cruz",
            "Barangay Santo Tomas",
            "09171234567",
            "juan@example.com"
        );

        const csms::ResidentRegistrationResult result =
            service.registerResident(invalidResident);

        assert(!result.success);
        assert(!result.resident.has_value());
        assert(!result.errors.empty());
    }
}

// Test 7 — Invalid Resident is not persisted
void testInvalidResidentIsNotPersisted()
{
    const std::string dbPath = makeTempDbPath("t04_not_persisted");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident invalidResident(
            "",
            "Dela Cruz",
            "Barangay Santo Tomas",
            "09171234567",
            "juan@example.com"
        );

        const int countBefore = countResidentsInDatabase(dbPath);

        const csms::ResidentRegistrationResult result =
            service.registerResident(invalidResident);

        const int countAfter = countResidentsInDatabase(dbPath);

        assert(!result.success);

        // Row count must not have changed — invalid Resident must not
        // have reached the repository.
        assert(countAfter == countBefore);
    }
}

// Test 8 — Registration result identifies the failing validation field
void testRegistrationReturnsValidationErrors()
{
    const std::string dbPath = makeTempDbPath("t04_errors");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentRegistrationService service(validator, repo);

        csms::Resident invalidResident(
            "",
            "Dela Cruz",
            "Barangay Santo Tomas",
            "09171234567",
            "juan@example.com"
        );

        const csms::ResidentRegistrationResult result =
            service.registerResident(invalidResident);

        assert(!result.success);

        // The errors vector must contain "firstName" somewhere.
        // std::find is used because there may be multiple errors.
        const auto it = std::find(
            result.errors.begin(),
            result.errors.end(),
            "firstName"
        );

        assert(it != result.errors.end());
    }
}

// ---------------------------------------------------------------------------
// T04 Drogon test wrapper — runs all T04 registration scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentRegistrationTest)
{
    testRegisterValidResident();
    testRegisteredResidentReceivesIdentifier();
    testRegisteredResidentIsPersisted();
    testRegisteredResidentInformationIsPreserved();
    testRegisteredResidentPreservesDefaultActiveStatus();
    testInvalidResidentRegistrationFails();
    testInvalidResidentIsNotPersisted();
    testRegistrationReturnsValidationErrors();
}

// ---------------------------------------------------------------------------
// T05 includes
// ---------------------------------------------------------------------------

#include "services/ResidentQueryService.h"

// ---------------------------------------------------------------------------
// T05 Helper — save a resident directly through the repository
// ---------------------------------------------------------------------------

static void saveResident(
    csms::ResidentRepository& repo,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& status = "Active")
{
    csms::Resident r(
        firstName,
        lastName,
        "Test Address",
        "09171234567",
        "test@example.com",
        status
    );
    repo.save(r);
}

// ---------------------------------------------------------------------------
// T05 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — List all persisted Residents
void testListAllResidents()
{
    const std::string dbPath = makeTempDbPath("t05_list_all");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan",  "Cruz");
        saveResident(repo, "Maria", "Santos");
        saveResident(repo, "Pedro", "Reyes");

        const std::vector<csms::Resident> result =
            service.listResidents();

        assert(result.size() == 3u);
    }
}

// Test 2 — Empty database returns empty collection
void testListResidentsWhenEmpty()
{
    const std::string dbPath = makeTempDbPath("t05_list_empty");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        const std::vector<csms::Resident> result =
            service.listResidents();

        // Must return empty vector, not crash or throw.
        assert(result.empty());
    }
}

// Test 3 — Listing uses required deterministic ordering
void testListResidentsOrdering()
{
    const std::string dbPath = makeTempDbPath("t05_ordering");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        // Insert in an order that differs from the expected display order.
        saveResident(repo, "Ana",   "Santos");
        saveResident(repo, "Pedro", "Cruz");
        saveResident(repo, "Maria", "Andres");
        saveResident(repo, "Juan",  "Cruz");

        const std::vector<csms::Resident> result =
            service.listResidents();

        assert(result.size() == 4u);

        // Expected order: Andres/Maria, Cruz/Juan, Cruz/Pedro, Santos/Ana
        assert(result[0].getLastName()  == "Andres");
        assert(result[0].getFirstName() == "Maria");

        assert(result[1].getLastName()  == "Cruz");
        assert(result[1].getFirstName() == "Juan");

        assert(result[2].getLastName()  == "Cruz");
        assert(result[2].getFirstName() == "Pedro");

        assert(result[3].getLastName()  == "Santos");
        assert(result[3].getFirstName() == "Ana");
    }
}

// Test 4 — Partial first name search is case-insensitive
void testSearchByPartialFirstNameCaseInsensitive()
{
    const std::string dbPath = makeTempDbPath("t05_firstname_search");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan", "Dela Cruz");

        // Mixed-case partial match against first name.
        const std::vector<csms::Resident> result =
            service.searchResidents("jUa");

        assert(result.size() == 1u);
        assert(result[0].getFirstName() == "Juan");
    }
}

// Test 5 — Partial last name search is case-insensitive
void testSearchByPartialLastNameCaseInsensitive()
{
    const std::string dbPath = makeTempDbPath("t05_lastname_search");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan", "Dela Cruz");

        // Mixed-case partial match against last name.
        const std::vector<csms::Resident> result =
            service.searchResidents("cRuZ");

        assert(result.size() == 1u);
        assert(result[0].getLastName() == "Dela Cruz");
    }
}

// Test 6 — Blank search returns all Residents
void testBlankSearchReturnsAllResidents()
{
    const std::string dbPath = makeTempDbPath("t05_blank_search");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan",  "Cruz");
        saveResident(repo, "Maria", "Santos");

        // Whitespace-only search term must behave like listResidents().
        const std::vector<csms::Resident> result =
            service.searchResidents("   ");

        assert(result.size() == 2u);
    }
}

// Test 7 — Search with no match returns empty collection
void testSearchWithNoMatchReturnsEmpty()
{
    const std::string dbPath = makeTempDbPath("t05_no_match");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan", "Cruz");

        const std::vector<csms::Resident> result =
            service.searchResidents("ZzzUnknownResident");

        assert(result.empty());
    }
}

// Test 8 — Search results preserve all Resident information
void testSearchPreservesResidentInformation()
{
    const std::string dbPath = makeTempDbPath("t05_search_info");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        csms::Resident r(
            "Juan",
            "Dela Cruz",
            "Barangay Santo Tomas",
            "09171234567",
            "juan@example.com",
            "Active"
        );
        repo.save(r);

        const std::vector<csms::Resident> result =
            service.searchResidents("Juan");

        assert(result.size() == 1u);
        assert(result[0].getFirstName()     == "Juan");
        assert(result[0].getLastName()      == "Dela Cruz");
        assert(result[0].getAddress()       == "Barangay Santo Tomas");
        // Leading zero must be preserved.
        assert(result[0].getContactNumber() == "09171234567");
        assert(result[0].getEmail()         == "juan@example.com");
        assert(result[0].getStatus()        == "Active");
        assert(result[0].getId()            > 0);
    }
}

// Test 9 — Both Active and Inactive Residents are included
void testListIncludesActiveAndInactiveResidents()
{
    const std::string dbPath = makeTempDbPath("t05_status_filter");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        saveResident(repo, "Juan",  "Cruz",   "Active");
        saveResident(repo, "Maria", "Santos", "Inactive");

        const std::vector<csms::Resident> result =
            service.listResidents();

        // Both must appear — T05 does not filter by status.
        assert(result.size() == 2u);

        bool foundActive   = false;
        bool foundInactive = false;

        for (const auto& resident : result)
        {
            if (resident.getStatus() == "Active")   foundActive   = true;
            if (resident.getStatus() == "Inactive") foundInactive = true;
        }

        assert(foundActive);
        assert(foundInactive);
    }
}

// Test 10 — Matching Resident appears only once even when both names match
void testSearchDoesNotDuplicateResident()
{
    const std::string dbPath = makeTempDbPath("t05_no_duplicate");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentQueryService service(repo);

        // Both first name and last name contain "Cruz".
        csms::Resident r(
            "Cruz",
            "Cruz",
            "Test Address",
            "09171234567",
            "cruz@example.com"
        );
        repo.save(r);

        const std::vector<csms::Resident> result =
            service.searchResidents("Cruz");

        // Must appear exactly once despite matching both columns.
        assert(result.size() == 1u);
    }
}

// ---------------------------------------------------------------------------
// T05 Drogon test wrapper — runs all T05 search and listing scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentSearchListTest)
{
    testListAllResidents();
    testListResidentsWhenEmpty();
    testListResidentsOrdering();
    testSearchByPartialFirstNameCaseInsensitive();
    testSearchByPartialLastNameCaseInsensitive();
    testBlankSearchReturnsAllResidents();
    testSearchWithNoMatchReturnsEmpty();
    testSearchPreservesResidentInformation();
    testListIncludesActiveAndInactiveResidents();
    testSearchDoesNotDuplicateResident();
}

// ---------------------------------------------------------------------------
// T06 includes
// ---------------------------------------------------------------------------

#include "services/ResidentUpdateService.h"

// ---------------------------------------------------------------------------
// T06 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — Valid Resident update succeeds
void testValidResidentUpdateSucceeds()
{
    const std::string dbPath = makeTempDbPath("t06_update_success");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        // Register an original Resident.
        csms::Resident original(
            "Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(original).getId();

        // Propose a valid update.
        csms::Resident proposed(
            "Juan Miguel", "Dela Cruz", "New Address", "09181234567",
            "juanmiguel@example.com"
        );

        const csms::ResidentUpdateResult result =
            service.updateResident(id, proposed);

        assert(result.success);
        assert(!result.notFound);
        assert(result.resident.has_value());
        assert(result.errors.empty());
    }
}

// Test 2 — Resident ID is preserved after update
void testResidentIdIsPreservedAfterUpdate()
{
    const std::string dbPath = makeTempDbPath("t06_id_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident original(
            "Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"
        );
        const int originalId = repo.save(original).getId();

        csms::Resident proposed(
            "Juan Miguel", "Dela Cruz", "New Address", "09181234567",
            "juanmiguel@example.com"
        );

        const csms::ResidentUpdateResult result =
            service.updateResident(originalId, proposed);

        assert(result.success);
        assert(result.resident.has_value());

        // The id must not change.
        assert(result.resident->getId() == originalId);
    }
}

// Test 3 — Permitted fields are persisted after update
void testPermittedFieldsArePersistedAfterUpdate()
{
    const std::string dbPath = makeTempDbPath("t06_fields_persisted");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident original(
            "Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(original).getId();

        csms::Resident proposed(
            "Maria", "Santos", "456 Rizal Ave", "09281234567",
            "maria.santos@example.com"
        );

        service.updateResident(id, proposed);

        // Retrieve through T03 findById to confirm persistence.
        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getFirstName()     == "Maria");
        assert(stored->getLastName()      == "Santos");
        assert(stored->getAddress()       == "456 Rizal Ave");
        assert(stored->getContactNumber() == "09281234567");
        assert(stored->getEmail()         == "maria.santos@example.com");
    }
}

// Test 4 — Resident status is preserved after update
void testResidentStatusIsPreservedAfterUpdate()
{
    const std::string dbPath = makeTempDbPath("t06_status_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        // Test both Active and Inactive.
        csms::Resident activeResident(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com",
            "Active"
        );
        const int activeId = repo.save(activeResident).getId();

        csms::Resident inactiveResident(
            "Maria", "Santos", "Address", "09281234567", "maria@example.com",
            "Inactive"
        );
        const int inactiveId = repo.save(inactiveResident).getId();

        csms::Resident proposed(
            "Updated", "Name", "New Address", "09381234567",
            "updated@example.com"
        );

        const csms::ResidentUpdateResult activeResult =
            service.updateResident(activeId, proposed);
        const csms::ResidentUpdateResult inactiveResult =
            service.updateResident(inactiveId, proposed);

        assert(activeResult.success);
        assert(activeResult.resident->getStatus() == "Active");

        assert(inactiveResult.success);
        assert(inactiveResult.resident->getStatus() == "Inactive");
    }
}

// Test 5 — Invalid update fails
void testInvalidUpdateFails()
{
    const std::string dbPath = makeTempDbPath("t06_invalid_update");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident original(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(original).getId();

        // Empty first name violates T02.
        csms::Resident invalidProposed(
            "", "Cruz", "Address", "09171234567", "juan@example.com"
        );

        const csms::ResidentUpdateResult result =
            service.updateResident(id, invalidProposed);

        assert(!result.success);
        assert(!result.notFound);
        assert(!result.errors.empty());
    }
}

// Test 6 — Invalid update does not modify persisted information
void testInvalidUpdateDoesNotModifyPersistedInformation()
{
    const std::string dbPath = makeTempDbPath("t06_invalid_no_change");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident original(
            "Juan", "Cruz", "Original Address", "09171234567",
            "juan@example.com"
        );
        const int id = repo.save(original).getId();

        // Invalid — empty first name.
        csms::Resident invalidProposed(
            "", "Cruz", "New Address", "09171234567", "juan@example.com"
        );

        service.updateResident(id, invalidProposed);

        // The original data must be unchanged.
        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getFirstName() == "Juan");
        assert(stored->getAddress()   == "Original Address");
    }
}

// Test 7 — Updating a nonexistent Resident is handled safely
void testUpdateNonexistentResidentIsHandledSafely()
{
    const std::string dbPath = makeTempDbPath("t06_not_found");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident proposed(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );

        // 999999 does not exist.
        const csms::ResidentUpdateResult result =
            service.updateResident(999999, proposed);

        assert(!result.success);
        assert(result.notFound);
        assert(!result.resident.has_value());
        assert(result.errors.empty());
    }
}

// Test 8 — Nonexistent update does not create a new Resident
void testNonexistentUpdateDoesNotCreateResident()
{
    const std::string dbPath = makeTempDbPath("t06_no_insert");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        const int countBefore =
            static_cast<int>(repo.findAll().size());

        csms::Resident proposed(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );

        service.updateResident(999999, proposed);

        const int countAfter =
            static_cast<int>(repo.findAll().size());

        // No new Resident must have been created.
        assert(countAfter == countBefore);
    }
}

// Test 9 — Updated Resident is visible through T05 querying
void testUpdatedResidentIsVisibleThroughT05Query()
{
    const std::string dbPath = makeTempDbPath("t06_t05_integration");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService updateService(validator, repo);
        csms::ResidentQueryService queryService(repo);

        csms::Resident original(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(original).getId();

        // Update to a completely different name.
        csms::Resident proposed(
            "Miguel", "Santos", "New Address", "09281234567",
            "miguel@example.com"
        );
        updateService.updateResident(id, proposed);

        // The old name must no longer appear.
        const auto oldResults = queryService.searchResidents("Juan Cruz");
        bool oldStillFound = false;
        for (const auto& r : oldResults)
        {
            if (r.getId() == id) oldStillFound = true;
        }
        assert(!oldStillFound);

        // The new name must be findable.
        const auto newResults = queryService.searchResidents("Miguel");
        bool newFound = false;
        for (const auto& r : newResults)
        {
            if (r.getId() == id) newFound = true;
        }
        assert(newFound);
    }
}

// Test 10 — Updated information and contact number are preserved
void testUpdatedInformationAndContactNumberArePreserved()
{
    const std::string dbPath = makeTempDbPath("t06_contact_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentValidator validator;
        csms::ResidentUpdateService service(validator, repo);

        csms::Resident original(
            "Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(original).getId();

        csms::Resident proposed(
            "Pedro", "Reyes", "789 Bonifacio St", "09181234567",
            "pedro.reyes@example.com"
        );

        const csms::ResidentUpdateResult result =
            service.updateResident(id, proposed);

        assert(result.success);

        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getFirstName()     == "Pedro");
        assert(stored->getLastName()      == "Reyes");
        assert(stored->getAddress()       == "789 Bonifacio St");
        // Leading zero must be preserved.
        assert(stored->getContactNumber() == "09181234567");
        assert(stored->getContactNumber()[0] == '0');
        assert(stored->getEmail()         == "pedro.reyes@example.com");
        // id and status must be unchanged.
        assert(stored->getId()            == id);
        assert(stored->getStatus()        == "Active");
    }
}

// ---------------------------------------------------------------------------
// T06 Drogon test wrapper — runs all T06 update scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentUpdateTest)
{
    testValidResidentUpdateSucceeds();
    testResidentIdIsPreservedAfterUpdate();
    testPermittedFieldsArePersistedAfterUpdate();
    testResidentStatusIsPreservedAfterUpdate();
    testInvalidUpdateFails();
    testInvalidUpdateDoesNotModifyPersistedInformation();
    testUpdateNonexistentResidentIsHandledSafely();
    testNonexistentUpdateDoesNotCreateResident();
    testUpdatedResidentIsVisibleThroughT05Query();
    testUpdatedInformationAndContactNumberArePreserved();
}

// ---------------------------------------------------------------------------
// T07 includes
// ---------------------------------------------------------------------------

#include "services/ResidentDeactivationService.h"

// ---------------------------------------------------------------------------
// T07 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — Active Resident can be deactivated
void testActiveResidentCanBeDeactivated()
{
    const std::string dbPath = makeTempDbPath("t07_deactivate");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com",
            "Active"
        );
        const int id = repo.save(r).getId();

        const csms::ResidentDeactivationResult result =
            service.deactivateResident(id);

        assert(result.success);
        assert(!result.notFound);
        assert(!result.alreadyInactive);
        assert(result.resident.has_value());
    }
}

// Test 2 — Status becomes Inactive in persistence
void testResidentStatusBecomesInactiveInPersistence()
{
    const std::string dbPath = makeTempDbPath("t07_status_inactive");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com",
            "Active"
        );
        const int id = repo.save(r).getId();

        service.deactivateResident(id);

        // Retrieve through T03 to confirm the status change is persisted.
        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getStatus() == "Inactive");
    }
}

// Test 3 — Resident ID is preserved after deactivation
void testResidentIdIsPreservedAfterDeactivation()
{
    const std::string dbPath = makeTempDbPath("t07_id_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );
        const int originalId = repo.save(r).getId();

        const csms::ResidentDeactivationResult result =
            service.deactivateResident(originalId);

        assert(result.success);
        assert(result.resident.has_value());
        assert(result.resident->getId() == originalId);
    }
}

// Test 4 — Resident information is preserved after deactivation
void testResidentInformationIsPreservedAfterDeactivation()
{
    const std::string dbPath = makeTempDbPath("t07_info_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r(
            "Juan", "Dela Cruz", "Barangay Santo Tomas",
            "09171234567", "juan@example.com", "Active"
        );
        const int id = repo.save(r).getId();

        service.deactivateResident(id);

        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getFirstName()     == "Juan");
        assert(stored->getLastName()      == "Dela Cruz");
        assert(stored->getAddress()       == "Barangay Santo Tomas");
        assert(stored->getContactNumber() == "09171234567");
        assert(stored->getEmail()         == "juan@example.com");
        // Only status changes.
        assert(stored->getStatus()        == "Inactive");
        assert(stored->getId()            == id);
    }
}

// Test 5 — Deactivated Resident remains persisted and retrievable
void testDeactivatedResidentRemainsPersistedAndRetrievable()
{
    const std::string dbPath = makeTempDbPath("t07_still_persisted");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );
        const int id = repo.save(r).getId();

        service.deactivateResident(id);

        // The record must still exist — not deleted.
        const std::optional<csms::Resident> stored = repo.findById(id);

        assert(stored.has_value());
        assert(stored->getStatus() == "Inactive");
    }
}

// Test 6 — Deactivated Resident remains available through T05
void testDeactivatedResidentRemainsAvailableThroughT05()
{
    const std::string dbPath = makeTempDbPath("t07_t05_integration");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService deactivationService(repo);
        csms::ResidentQueryService queryService(repo);

        csms::Resident r(
            "Maria", "Santos", "Address", "09171234567", "maria@example.com",
            "Active"
        );
        const int id = repo.save(r).getId();

        deactivationService.deactivateResident(id);

        // T05 listing must still include Inactive Residents.
        const auto listed = queryService.listResidents();
        bool found = false;
        for (const auto& res : listed)
        {
            if (res.getId() == id)
            {
                found = true;
                assert(res.getStatus() == "Inactive");
            }
        }
        assert(found);

        // T05 search must also still find the Resident.
        const auto searched = queryService.searchResidents("Maria");
        bool searchFound = false;
        for (const auto& res : searched)
        {
            if (res.getId() == id) searchFound = true;
        }
        assert(searchFound);
    }
}

// Test 7 — Already-Inactive Resident is handled safely
void testAlreadyInactiveResidentIsHandledSafely()
{
    const std::string dbPath = makeTempDbPath("t07_already_inactive");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        // Start with an already-Inactive Resident.
        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com",
            "Inactive"
        );
        const int id = repo.save(r).getId();

        const csms::ResidentDeactivationResult result =
            service.deactivateResident(id);

        // Must succeed safely — no crash, no new record, no data change.
        assert(result.success);
        assert(result.alreadyInactive);
        assert(!result.notFound);
        assert(result.resident.has_value());
        assert(result.resident->getStatus() == "Inactive");
        assert(result.resident->getId()     == id);
    }
}

// Test 8 — Nonexistent Resident is handled safely
void testNonexistentResidentIsHandledSafely()
{
    const std::string dbPath = makeTempDbPath("t07_not_found");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        const csms::ResidentDeactivationResult result =
            service.deactivateResident(999999);

        assert(!result.success);
        assert(result.notFound);
        assert(!result.alreadyInactive);
        assert(!result.resident.has_value());
    }
}

// Test 9 — Nonexistent deactivation does not create or delete records
void testNonexistentDeactivationDoesNotCreateOrDeleteRecords()
{
    const std::string dbPath = makeTempDbPath("t07_no_side_effects");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        // Save one real Resident first.
        csms::Resident r(
            "Juan", "Cruz", "Address", "09171234567", "juan@example.com"
        );
        const int realId = repo.save(r).getId();

        const int countBefore =
            static_cast<int>(repo.findAll().size());

        // Attempt to deactivate a nonexistent id.
        service.deactivateResident(999999);

        const int countAfter =
            static_cast<int>(repo.findAll().size());

        // Count must be unchanged — no insert or delete.
        assert(countAfter == countBefore);

        // The real Resident must be unaffected.
        const std::optional<csms::Resident> stored = repo.findById(realId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Active");
    }
}

// Test 10 — Deactivating one Resident does not affect another
void testDeactivatingOneResidentDoesNotAffectAnother()
{
    const std::string dbPath = makeTempDbPath("t07_isolation");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository repo(db);
        csms::ResidentDeactivationService service(repo);

        csms::Resident r1(
            "Juan", "Cruz", "Address 1", "09171234567", "juan@example.com",
            "Active"
        );
        csms::Resident r2(
            "Maria", "Santos", "Address 2", "09281234567", "maria@example.com",
            "Active"
        );

        const int id1 = repo.save(r1).getId();
        const int id2 = repo.save(r2).getId();

        // Deactivate only Resident 1.
        service.deactivateResident(id1);

        const std::optional<csms::Resident> stored1 = repo.findById(id1);
        const std::optional<csms::Resident> stored2 = repo.findById(id2);

        assert(stored1.has_value());
        assert(stored1->getStatus() == "Inactive");

        // Resident 2 must remain Active and fully unchanged.
        assert(stored2.has_value());
        assert(stored2->getStatus()    == "Active");
        assert(stored2->getFirstName() == "Maria");
        assert(stored2->getLastName()  == "Santos");
    }
}

// ---------------------------------------------------------------------------
// T07 Drogon test wrapper — runs all T07 deactivation scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentDeactivationTest)
{
    testActiveResidentCanBeDeactivated();
    testResidentStatusBecomesInactiveInPersistence();
    testResidentIdIsPreservedAfterDeactivation();
    testResidentInformationIsPreservedAfterDeactivation();
    testDeactivatedResidentRemainsPersistedAndRetrievable();
    testDeactivatedResidentRemainsAvailableThroughT05();
    testAlreadyInactiveResidentIsHandledSafely();
    testNonexistentResidentIsHandledSafely();
    testNonexistentDeactivationDoesNotCreateOrDeleteRecords();
    testDeactivatingOneResidentDoesNotAffectAnother();
}

// ---------------------------------------------------------------------------
// T08 includes
// ---------------------------------------------------------------------------

#include "models/ServiceRequest.h"

// ---------------------------------------------------------------------------
// T08 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — Service Request can be created
void testServiceRequestCanBeCreated()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Request for employment requirement",
        "2026-09-25"
    );

    // Construction must succeed — verified by accessing any field.
    assert(request.getResidentId() == 25);
}

// Test 2 — Service Request information is accessible
void testServiceRequestInformationIsAccessible()
{
    csms::ServiceRequest request(
        25,
        "Certificate Request",
        "Requesting certificate of residency",
        "2026-09-25"
    );

    assert(request.getResidentId()    == 25);
    assert(request.getServiceType()   == "Certificate Request");
    assert(request.getDescription()   == "Requesting certificate of residency");
    assert(request.getDateRequested() == "2026-09-25");
}

// Test 3 — Resident ID is preserved
void testServiceRequestResidentIdIsPreserved()
{
    csms::ServiceRequest request(
        25,
        "Community Assistance",
        "Requesting community assistance",
        "2026-09-25"
    );

    // residentId must not be replaced or modified.
    assert(request.getResidentId() == 25);
}

// Test 4 — New Service Request has an unassigned ID
void testNewServiceRequestHasUnassignedId()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Test description",
        "2026-09-25"
    );

    // id must be std::nullopt before persistence assigns one.
    assert(!request.getId().has_value());
}

// Test 5 — New Service Request defaults to Pending
void testNewServiceRequestDefaultsToPending()
{
    // Create without explicitly supplying a status.
    csms::ServiceRequest request(
        25,
        "Permit Request",
        "Requesting a permit",
        "2026-09-25"
    );

    assert(request.getStatus() == "Pending");
}

// Test 6 — Service Request information is independent between objects
void testServiceRequestInformationIsIndependentBetweenObjects()
{
    csms::ServiceRequest request1(
        10,
        "Barangay Clearance",
        "First request description",
        "2026-09-01"
    );

    csms::ServiceRequest request2(
        20,
        "Certificate Request",
        "Second request description",
        "2026-09-15"
    );

    // Each object must preserve its own data independently.
    assert(request1.getResidentId()    == 10);
    assert(request1.getServiceType()   == "Barangay Clearance");
    assert(request1.getDescription()   == "First request description");
    assert(request1.getDateRequested() == "2026-09-01");

    assert(request2.getResidentId()    == 20);
    assert(request2.getServiceType()   == "Certificate Request");
    assert(request2.getDescription()   == "Second request description");
    assert(request2.getDateRequested() == "2026-09-15");

    // Ensure they do not share state.
    assert(request1.getResidentId()  != request2.getResidentId());
    assert(request1.getServiceType() != request2.getServiceType());
}

// ---------------------------------------------------------------------------
// T08 Drogon test wrapper — runs all T08 Service Request domain scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestDomainTest)
{
    testServiceRequestCanBeCreated();
    testServiceRequestInformationIsAccessible();
    testServiceRequestResidentIdIsPreserved();
    testNewServiceRequestHasUnassignedId();
    testNewServiceRequestDefaultsToPending();
    testServiceRequestInformationIsIndependentBetweenObjects();
}

// ---------------------------------------------------------------------------
// T09 includes
// ---------------------------------------------------------------------------

#include "models/ServiceRequestValidator.h"
#include "repositories/ServiceRequestRepository.h"
#include "services/ServiceRequestSubmissionService.h"
#include "services/ServiceRequestSubmissionResult.h"

// ---------------------------------------------------------------------------
// T09 Helpers
// ---------------------------------------------------------------------------

// Returns a valid ServiceRequest for an already-persisted Resident.
static csms::ServiceRequest makeValidServiceRequest(int residentId)
{
    return csms::ServiceRequest(
        residentId,
        "Barangay Clearance",
        "Requesting barangay clearance for employment requirements.",
        "2026-09-25"
        // status defaults to "Pending"
    );
}

// Saves an Active Resident directly and returns its generated id.
static int saveActiveResident(csms::ResidentRepository& repo)
{
    csms::Resident r(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
        // status defaults to "Active"
    );
    return repo.save(r).getId();
}

// Saves an Inactive Resident directly and returns its generated id.
static int saveInactiveResident(csms::ResidentRepository& repo)
{
    csms::Resident r(
        "Maria",
        "Santos",
        "456 Rizal Ave",
        "09281234567",
        "maria@example.com",
        "Inactive"
    );
    return repo.save(r).getId();
}

// Counts rows in service_requests using a raw sqlite3 connection.
// Used by the "does not reach persistence" tests.
static int countServiceRequests(const std::string& dbPath)
{
    sqlite3* db = nullptr;
    sqlite3_open(dbPath.c_str(), &db);

    // Make sure the table exists before counting.
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS service_requests ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "resident_id INTEGER NOT NULL,"
        "service_type TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "date_requested TEXT NOT NULL,"
        "status TEXT NOT NULL DEFAULT 'Pending');",
        nullptr, nullptr, nullptr);

    const char* sql = "SELECT COUNT(*) FROM service_requests;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_step(stmt);

    const int count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return count;
}

// ---------------------------------------------------------------------------
// T09 — Service Request Validation Test Functions
// ---------------------------------------------------------------------------

// Test 1 — Valid Service Request passes validation
void testValidServiceRequestPassesValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Requesting barangay clearance for employment requirements.",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;

    assert(validator.isValid(request));
    assert(validator.validate(request).empty());
}

// Test 2 — Assigned ID fails validation (not a new submission)
void testAssignedIdFailsValidation()
{
    // Use the with-id constructor — simulates a request already persisted.
    csms::ServiceRequest request(
        17,   // id already assigned
        25,
        "Barangay Clearance",
        "Some description",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "id"));
}

// Test 3 — Zero residentId fails validation
void testZeroResidentIdFailsValidation()
{
    csms::ServiceRequest request(
        0,   // invalid — must be positive
        "Barangay Clearance",
        "Some description",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "residentId"));
}

// Test 4 — Negative residentId fails validation
void testNegativeResidentIdFailsValidation()
{
    csms::ServiceRequest request(
        -5,
        "Barangay Clearance",
        "Some description",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "residentId"));
}

// Test 5 — Blank serviceType fails validation
void testBlankServiceTypeFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "",   // blank
        "Some description",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "serviceType"));
}

// Test 6 — Whitespace-only serviceType fails validation
void testWhitespaceServiceTypeFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "   ",   // whitespace only
        "Some description",
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "serviceType"));
}

// Test 7 — Blank description fails validation
void testBlankDescriptionFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "",   // blank
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "description"));
}

// Test 8 — Whitespace-only description fails validation
void testWhitespaceDescriptionFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "   ",   // whitespace only
        "2026-09-25"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "description"));
}

// Test 9 — Blank dateRequested fails validation
void testBlankDateRequestedFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Some description",
        ""   // blank date
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "dateRequested"));
}

// Test 10 — Non-Pending status fails validation
void testNonPendingStatusFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Some description",
        "2026-09-25",
        "Completed"   // must not start as Completed
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "status"));
}

// Test 11 — In Progress initial status fails validation
void testInProgressInitialStatusFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Some description",
        "2026-09-25",
        "In Progress"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "status"));
}

// Test 12 — Cancelled initial status fails validation
void testCancelledInitialStatusFailsValidation()
{
    csms::ServiceRequest request(
        25,
        "Barangay Clearance",
        "Some description",
        "2026-09-25",
        "Cancelled"
    );

    csms::ServiceRequestValidator validator;
    const auto errors = validator.validate(request);

    assert(!validator.isValid(request));
    assert(containsValidationError(errors, "status"));
}

// ---------------------------------------------------------------------------
// T09 — Service Request Validation DROGON_TEST wrapper
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestValidationTest)
{
    testValidServiceRequestPassesValidation();
    testAssignedIdFailsValidation();
    testZeroResidentIdFailsValidation();
    testNegativeResidentIdFailsValidation();
    testBlankServiceTypeFailsValidation();
    testWhitespaceServiceTypeFailsValidation();
    testBlankDescriptionFailsValidation();
    testWhitespaceDescriptionFailsValidation();
    testBlankDateRequestedFailsValidation();
    testNonPendingStatusFailsValidation();
    testInProgressInitialStatusFailsValidation();
    testCancelledInitialStatusFailsValidation();
}

// ---------------------------------------------------------------------------
// T09 — Service Request Submission Test Functions
// ---------------------------------------------------------------------------

// Test 1 — Valid Service Request submission succeeds
void testValidServiceRequestSubmissionSucceeds()
{
    const std::string dbPath = makeTempDbPath("t09_submit_success");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);
        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(!result.residentNotFound);
        assert(!result.residentInactive);
        assert(result.serviceRequest.has_value());
        assert(result.errors.empty());
    }
}

// Test 2 — Submitted Service Request receives a generated ID
void testSubmittedServiceRequestReceivesGeneratedId()
{
    const std::string dbPath = makeTempDbPath("t09_generated_id");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);
        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);

        // Before submission the id must be unassigned.
        assert(!request.getId().has_value());

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(result.serviceRequest.has_value());

        // After successful persistence SQLite must have assigned a positive id.
        assert(result.serviceRequest->getId().has_value());
        assert(result.serviceRequest->getId().value() > 0);
    }
}

// Test 3 — Submitted Service Request is persisted and retrievable
void testSubmittedServiceRequestIsPersistedAndRetrievable()
{
    const std::string dbPath = makeTempDbPath("t09_retrievable");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);
        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(result.serviceRequest.has_value());

        const int generatedId = result.serviceRequest->getId().value();

        // Must be retrievable from the repository by its generated id.
        const std::optional<csms::ServiceRequest> found =
            requestRepo.findById(generatedId);

        assert(found.has_value());
        assert(found->getId().has_value());
        assert(found->getId().value() == generatedId);
    }
}

// Test 4 — Submitted Service Request information is preserved
void testSubmittedServiceRequestInformationIsPreserved()
{
    const std::string dbPath = makeTempDbPath("t09_info_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        csms::ServiceRequest request(
            residentId,
            "Certificate Request",
            "Requesting certificate of residency.",
            "2026-09-20"
        );

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(result.serviceRequest.has_value());

        const int id = result.serviceRequest->getId().value();
        const std::optional<csms::ServiceRequest> found =
            requestRepo.findById(id);

        assert(found.has_value());
        assert(found->getResidentId()    == residentId);
        assert(found->getServiceType()   == "Certificate Request");
        assert(found->getDescription()   == "Requesting certificate of residency.");
        assert(found->getDateRequested() == "2026-09-20");
        assert(found->getStatus()        == "Pending");
    }
}

// Test 5 — Submitted Service Request status is Pending
void testSubmittedServiceRequestStatusIsPending()
{
    const std::string dbPath = makeTempDbPath("t09_status_pending");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);
        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(result.serviceRequest.has_value());
        assert(result.serviceRequest->getStatus() == "Pending");

        // Also verify from the repository directly.
        const int id = result.serviceRequest->getId().value();
        const std::optional<csms::ServiceRequest> found =
            requestRepo.findById(id);

        assert(found.has_value());
        assert(found->getStatus() == "Pending");
    }
}

// Test 6 — Blank serviceType submission fails
void testBlankServiceTypeSubmissionFails()
{
    const std::string dbPath = makeTempDbPath("t09_blank_service_type");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        csms::ServiceRequest request(
            residentId,
            "",   // blank serviceType
            "Some description",
            "2026-09-25"
        );

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(!result.success);
        assert(!result.errors.empty());
        assert(containsValidationError(result.errors, "serviceType"));
        assert(!result.serviceRequest.has_value());
    }
}

// Test 7 — Blank description submission fails
void testBlankDescriptionSubmissionFails()
{
    const std::string dbPath = makeTempDbPath("t09_blank_description");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        csms::ServiceRequest request(
            residentId,
            "Barangay Clearance",
            "",   // blank description
            "2026-09-25"
        );

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(!result.success);
        assert(!result.errors.empty());
        assert(containsValidationError(result.errors, "description"));
        assert(!result.serviceRequest.has_value());
    }
}

// Test 8 — Invalid request does not reach persistence
void testInvalidRequestDoesNotReachPersistence()
{
    const std::string dbPath = makeTempDbPath("t09_no_persist_invalid");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        // Both serviceType and description are blank — must fail validation.
        csms::ServiceRequest request(
            residentId,
            "   ",   // whitespace-only serviceType
            "",       // blank description
            "2026-09-25"
        );

        const int countBefore = countServiceRequests(dbPath);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        const int countAfter = countServiceRequests(dbPath);

        assert(!result.success);
        // Row count must not have increased — invalid request never persisted.
        assert(countAfter == countBefore);
    }
}

// Test 9 — Nonexistent Resident prevents submission
void testNonexistentResidentPreventsSubmission()
{
    const std::string dbPath = makeTempDbPath("t09_resident_not_found");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        // Use a structurally valid residentId that has never been persisted.
        csms::ServiceRequest request(
            999999,
            "Barangay Clearance",
            "Requesting barangay clearance for employment requirements.",
            "2026-09-25"
        );

        const int countBefore = countServiceRequests(dbPath);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        const int countAfter = countServiceRequests(dbPath);

        assert(!result.success);
        assert(result.residentNotFound);
        assert(!result.residentInactive);
        assert(!result.serviceRequest.has_value());
        // No Service Request must have been persisted.
        assert(countAfter == countBefore);
    }
}

// Test 10 — Inactive Resident cannot submit a new Service Request
void testInactiveResidentCannotSubmit()
{
    const std::string dbPath = makeTempDbPath("t09_inactive_resident");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int inactiveId = saveInactiveResident(residentRepo);

        csms::ServiceRequest request(
            inactiveId,
            "Barangay Clearance",
            "Requesting barangay clearance for employment requirements.",
            "2026-09-25"
        );

        const int countBefore = countServiceRequests(dbPath);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        const int countAfter = countServiceRequests(dbPath);

        assert(!result.success);
        assert(!result.residentNotFound);
        assert(result.residentInactive);
        assert(!result.serviceRequest.has_value());
        // No Service Request must have been persisted.
        assert(countAfter == countBefore);

        // The Resident itself must remain Inactive and unchanged.
        const std::optional<csms::Resident> stored =
            residentRepo.findById(inactiveId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Inactive");
    }
}

// Test 11 — Non-Pending initial status is rejected
void testNonPendingInitialStatusIsRejected()
{
    const std::string dbPath = makeTempDbPath("t09_non_pending");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        // Attempt to submit a request that starts as Completed.
        csms::ServiceRequest request(
            residentId,
            "Barangay Clearance",
            "Some description",
            "2026-09-25",
            "Completed"   // not Pending — must be rejected
        );

        const int countBefore = countServiceRequests(dbPath);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        const int countAfter = countServiceRequests(dbPath);

        assert(!result.success);
        assert(!result.errors.empty());
        assert(containsValidationError(result.errors, "status"));
        // Must not have reached persistence.
        assert(countAfter == countBefore);
    }
}

// Test 12 — Service Request persists across repository instances
void testServiceRequestPersistsAcrossRepositoryInstances()
{
    const std::string dbPath = makeTempDbPath("t09_cross_instance");

    int savedId = 0;

    // First scope: submit and record the generated id, then close everything.
    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);
        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        assert(result.success);
        assert(result.serviceRequest.has_value());
        savedId = result.serviceRequest->getId().value();
        assert(savedId > 0);
        // db destructor closes the connection here.
    }

    // Second scope: open the same file with a brand-new instance.
    {
        csms::Database db2(dbPath);
        csms::ServiceRequestRepository requestRepo2(db2);

        const std::optional<csms::ServiceRequest> found =
            requestRepo2.findById(savedId);

        // The record must still exist — it is in the SQLite file, not memory.
        assert(found.has_value());
        assert(found->getId().has_value());
        assert(found->getId().value() == savedId);
        assert(found->getStatus()     == "Pending");
    }
}

// Test 13 — Submission does not modify the Resident
void testSubmissionDoesNotModifyTheResident()
{
    const std::string dbPath = makeTempDbPath("t09_resident_unchanged");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        // Capture the Resident state before submission.
        const std::optional<csms::Resident> before =
            residentRepo.findById(residentId);
        assert(before.has_value());

        const csms::ServiceRequest request =
            makeValidServiceRequest(residentId);
        service.submitRequest(request);

        // Retrieve the Resident again after submission.
        const std::optional<csms::Resident> after =
            residentRepo.findById(residentId);
        assert(after.has_value());

        // Every field must be identical — submission must not alter the Resident.
        assert(after->getId()            == before->getId());
        assert(after->getFirstName()     == before->getFirstName());
        assert(after->getLastName()      == before->getLastName());
        assert(after->getAddress()       == before->getAddress());
        assert(after->getContactNumber() == before->getContactNumber());
        assert(after->getEmail()         == before->getEmail());
        assert(after->getStatus()        == before->getStatus());
        assert(after->getStatus()        == "Active");
    }
}

// Test 14 — Date validation: blank dateRequested is rejected by submission
void testBlankDateRequestedIsRejectedBySubmission()
{
    const std::string dbPath = makeTempDbPath("t09_blank_date");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService service(
            validator, residentRepo, requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        csms::ServiceRequest request(
            residentId,
            "Barangay Clearance",
            "Requesting barangay clearance.",
            ""   // blank dateRequested
        );

        const int countBefore = countServiceRequests(dbPath);

        const csms::ServiceRequestSubmissionResult result =
            service.submitRequest(request);

        const int countAfter = countServiceRequests(dbPath);

        assert(!result.success);
        assert(!result.errors.empty());
        assert(containsValidationError(result.errors, "dateRequested"));
        // Must not have been persisted.
        assert(countAfter == countBefore);
    }
}

// ---------------------------------------------------------------------------
// T09 — Service Request Submission DROGON_TEST wrapper
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestSubmissionTest)
{
    testValidServiceRequestSubmissionSucceeds();
    testSubmittedServiceRequestReceivesGeneratedId();
    testSubmittedServiceRequestIsPersistedAndRetrievable();
    testSubmittedServiceRequestInformationIsPreserved();
    testSubmittedServiceRequestStatusIsPending();
    testBlankServiceTypeSubmissionFails();
    testBlankDescriptionSubmissionFails();
    testInvalidRequestDoesNotReachPersistence();
    testNonexistentResidentPreventsSubmission();
    testInactiveResidentCannotSubmit();
    testNonPendingInitialStatusIsRejected();
    testServiceRequestPersistsAcrossRepositoryInstances();
    testSubmissionDoesNotModifyTheResident();
    testBlankDateRequestedIsRejectedBySubmission();
}

// ---------------------------------------------------------------------------
// T10 includes
// ---------------------------------------------------------------------------

#include "services/ServiceRequestStatusService.h"
#include "services/ServiceRequestStatusResult.h"

// ---------------------------------------------------------------------------
// T10 Helpers
// ---------------------------------------------------------------------------

// Submits a valid Service Request for an Active Resident and returns the
// generated Service Request id. Used as the starting point for all T10 tests.
static int submitNewRequest(
    csms::ResidentRepository& residentRepo,
    csms::ServiceRequestRepository& requestRepo)
{
    const int residentId = saveActiveResident(residentRepo);

    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(
        validator, residentRepo, requestRepo);

    const csms::ServiceRequest request = makeValidServiceRequest(residentId);
    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(result.success);
    assert(result.serviceRequest.has_value());
    return result.serviceRequest->getId().value();
}

// Advances a Service Request from Pending to In Progress using the
// legitimate T10 workflow. Returns the updated Service Request id.
static int advanceToInProgress(
    int requestId,
    csms::ServiceRequestStatusService& statusService)
{
    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(requestId, "In Progress");
    assert(result.success);
    return requestId;
}

// ---------------------------------------------------------------------------
// T10 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — Pending can move to In Progress
void testPendingCanMoveToInProgress()
{
    const std::string dbPath = makeTempDbPath("t10_pending_to_inprogress");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "In Progress");

        assert(result.success);
        assert(!result.notFound);
        assert(!result.unsupportedStatus);
        assert(!result.invalidTransition);
        assert(result.serviceRequest.has_value());
        assert(result.serviceRequest->getStatus() == "In Progress");

        // Verify the change is persisted.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "In Progress");
    }
}

// Test 2 — Pending can move to Cancelled
void testPendingCanMoveToCancelled()
{
    const std::string dbPath = makeTempDbPath("t10_pending_to_cancelled");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Cancelled");

        assert(result.success);
        assert(result.serviceRequest.has_value());
        assert(result.serviceRequest->getStatus() == "Cancelled");

        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Cancelled");
    }
}

// Test 3 — In Progress can move to Completed
void testInProgressCanMoveToCompleted()
{
    const std::string dbPath = makeTempDbPath("t10_inprogress_to_completed");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);
        advanceToInProgress(requestId, statusService);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Completed");

        assert(result.success);
        assert(result.serviceRequest.has_value());
        assert(result.serviceRequest->getStatus() == "Completed");

        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Completed");
    }
}

// Test 4 — In Progress can move to Cancelled
void testInProgressCanMoveToCancelled()
{
    const std::string dbPath = makeTempDbPath("t10_inprogress_to_cancelled");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);
        advanceToInProgress(requestId, statusService);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Cancelled");

        assert(result.success);
        assert(result.serviceRequest.has_value());
        assert(result.serviceRequest->getStatus() == "Cancelled");

        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Cancelled");
    }
}

// Test 5 — Pending cannot move directly to Completed
void testPendingCannotMoveDirectlyToCompleted()
{
    const std::string dbPath = makeTempDbPath("t10_pending_not_completed");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Completed");

        assert(!result.success);
        assert(result.invalidTransition);

        // Persistence must remain Pending.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Pending");
    }
}

// Test 6 — In Progress cannot return to Pending
void testInProgressCannotReturnToPending()
{
    const std::string dbPath = makeTempDbPath("t10_inprogress_not_pending");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);
        advanceToInProgress(requestId, statusService);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Pending");

        assert(!result.success);
        assert(result.invalidTransition);

        // Persistence must remain In Progress.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "In Progress");
    }
}

// Test 7 — Completed is terminal
void testCompletedIsTerminal()
{
    const std::string dbPath = makeTempDbPath("t10_completed_terminal");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);
        advanceToInProgress(requestId, statusService);
        statusService.manageStatus(requestId, "Completed");

        // Attempt all three possible transitions away from Completed.
        const auto toPending =
            statusService.manageStatus(requestId, "Pending");
        const auto toInProgress =
            statusService.manageStatus(requestId, "In Progress");
        const auto toCancelled =
            statusService.manageStatus(requestId, "Cancelled");

        assert(!toPending.success    && toPending.invalidTransition);
        assert(!toInProgress.success && toInProgress.invalidTransition);
        assert(!toCancelled.success  && toCancelled.invalidTransition);

        // Persistence must remain Completed.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Completed");
    }
}

// Test 8 — Cancelled is terminal
void testCancelledIsTerminal()
{
    const std::string dbPath = makeTempDbPath("t10_cancelled_terminal");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);
        statusService.manageStatus(requestId, "Cancelled");

        // Attempt all three possible transitions away from Cancelled.
        const auto toPending =
            statusService.manageStatus(requestId, "Pending");
        const auto toInProgress =
            statusService.manageStatus(requestId, "In Progress");
        const auto toCompleted =
            statusService.manageStatus(requestId, "Completed");

        assert(!toPending.success    && toPending.invalidTransition);
        assert(!toInProgress.success && toInProgress.invalidTransition);
        assert(!toCompleted.success  && toCompleted.invalidTransition);

        // Persistence must remain Cancelled.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Cancelled");
    }
}

// Test 9 — Unsupported status is rejected
void testUnsupportedStatusIsRejected()
{
    const std::string dbPath = makeTempDbPath("t10_unsupported_status");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Approved");

        assert(!result.success);
        assert(result.unsupportedStatus);
        assert(!result.invalidTransition);

        // Persistence must remain Pending.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Pending");
    }
}

// Test 10 — Nonexistent Service Request is handled safely
void testNonexistentServiceRequestIsHandledSafely()
{
    const std::string dbPath = makeTempDbPath("t10_not_found");

    {
        csms::Database db(dbPath);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        // 999999 has never been persisted.
        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(999999, "In Progress");

        assert(!result.success);
        assert(result.notFound);
        assert(!result.unsupportedStatus);
        assert(!result.invalidTransition);
        assert(!result.serviceRequest.has_value());

        // No new record must have been created.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(999999);
        assert(!stored.has_value());
    }
}

// Test 11 — Successful transition preserves Service Request information
void testSuccessfulTransitionPreservesServiceRequestInformation()
{
    const std::string dbPath = makeTempDbPath("t10_info_preserved");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        // Capture all fields before the transition.
        const std::optional<csms::ServiceRequest> before =
            requestRepo.findById(requestId);
        assert(before.has_value());

        statusService.manageStatus(requestId, "In Progress");

        // Retrieve the updated record.
        const std::optional<csms::ServiceRequest> after =
            requestRepo.findById(requestId);
        assert(after.has_value());

        // Only status may change.
        assert(after->getId()            == before->getId());
        assert(after->getResidentId()    == before->getResidentId());
        assert(after->getServiceType()   == before->getServiceType());
        assert(after->getDescription()   == before->getDescription());
        assert(after->getDateRequested() == before->getDateRequested());

        // Status must have changed.
        assert(after->getStatus() == "In Progress");
        assert(before->getStatus() == "Pending");
    }
}

// Test 12 — Invalid transition does not modify persistence
void testInvalidTransitionDoesNotModifyPersistence()
{
    const std::string dbPath = makeTempDbPath("t10_invalid_no_modify");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        // Capture all fields before the invalid attempt.
        const std::optional<csms::ServiceRequest> before =
            requestRepo.findById(requestId);
        assert(before.has_value());

        // Attempt invalid transition: Pending -> Completed.
        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Completed");
        assert(!result.success);
        assert(result.invalidTransition);

        // Every field must remain unchanged.
        const std::optional<csms::ServiceRequest> after =
            requestRepo.findById(requestId);
        assert(after.has_value());

        assert(after->getId()            == before->getId());
        assert(after->getResidentId()    == before->getResidentId());
        assert(after->getServiceType()   == before->getServiceType());
        assert(after->getDescription()   == before->getDescription());
        assert(after->getDateRequested() == before->getDateRequested());
        assert(after->getStatus()        == before->getStatus());
        assert(after->getStatus()        == "Pending");
    }
}

// Test 13 — Same-status request is rejected
void testSameStatusRequestIsRejected()
{
    const std::string dbPath = makeTempDbPath("t10_same_status");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        // Pending -> Pending must be rejected.
        const csms::ServiceRequestStatusResult pendingToPending =
            statusService.manageStatus(requestId, "Pending");

        assert(!pendingToPending.success);
        assert(pendingToPending.invalidTransition);

        // Persistence must still be Pending.
        const std::optional<csms::ServiceRequest> stored =
            requestRepo.findById(requestId);
        assert(stored.has_value());
        assert(stored->getStatus() == "Pending");

        // Also verify for In Progress -> In Progress.
        advanceToInProgress(requestId, statusService);

        const csms::ServiceRequestStatusResult inProgressToInProgress =
            statusService.manageStatus(requestId, "In Progress");

        assert(!inProgressToInProgress.success);
        assert(inProgressToInProgress.invalidTransition);

        const std::optional<csms::ServiceRequest> stored2 =
            requestRepo.findById(requestId);
        assert(stored2.has_value());
        assert(stored2->getStatus() == "In Progress");
    }
}

// ---------------------------------------------------------------------------
// Student-Designed Test
//
// Test Name: testMultipleSequentialTransitionsFollowCorrectWorkflow
//
// What it verifies:
//   A single Service Request is walked through the complete forward workflow:
//   Pending -> In Progress -> Completed. After each step the persisted status
//   is confirmed, and a subsequent invalid attempt (Completed -> Cancelled) is
//   verified to leave the terminal state intact. This proves that the
//   transition rules hold correctly across three consecutive operations on the
//   same request, not just in isolated single-step tests.
//
// Why I added this test:
//   Individual required tests each verify a single hop in isolation. A bug
//   could theoretically pass all individual tests but fail when multiple
//   transitions are applied sequentially to the same row (e.g. if updateStatus
//   accidentally resets fields or targets the wrong row). This test exercises
//   the full lifecycle path end-to-end, which the required tests do not
//   collectively cover as a sequence on one record.
// ---------------------------------------------------------------------------

void testMultipleSequentialTransitionsFollowCorrectWorkflow()
{
    const std::string dbPath = makeTempDbPath("t10_sequential_transitions");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        const int requestId = submitNewRequest(residentRepo, requestRepo);

        // Step 1 — Pending -> In Progress
        {
            const csms::ServiceRequestStatusResult r =
                statusService.manageStatus(requestId, "In Progress");
            assert(r.success);
            const std::optional<csms::ServiceRequest> s =
                requestRepo.findById(requestId);
            assert(s.has_value());
            assert(s->getStatus() == "In Progress");
        }

        // Step 2 — In Progress -> Completed
        {
            const csms::ServiceRequestStatusResult r =
                statusService.manageStatus(requestId, "Completed");
            assert(r.success);
            const std::optional<csms::ServiceRequest> s =
                requestRepo.findById(requestId);
            assert(s.has_value());
            assert(s->getStatus() == "Completed");
        }

        // Step 3 — Completed -> Cancelled must fail (terminal state)
        {
            const csms::ServiceRequestStatusResult r =
                statusService.manageStatus(requestId, "Cancelled");
            assert(!r.success);
            assert(r.invalidTransition);
            const std::optional<csms::ServiceRequest> s =
                requestRepo.findById(requestId);
            assert(s.has_value());
            assert(s->getStatus() == "Completed");
        }

        // Non-status fields must remain intact throughout all transitions.
        const std::optional<csms::ServiceRequest> final =
            requestRepo.findById(requestId);
        assert(final.has_value());
        assert(final->getId().has_value());
        assert(final->getId().value() == requestId);
        assert(final->getServiceType()   == "Barangay Clearance");
        assert(final->getDescription()   ==
               "Requesting barangay clearance for employment requirements.");
        assert(final->getDateRequested() == "2026-09-25");
    }
}

// ---------------------------------------------------------------------------
// T10 — Service Request Status Management DROGON_TEST wrapper
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestStatusTest)
{
    testPendingCanMoveToInProgress();
    testPendingCanMoveToCancelled();
    testInProgressCanMoveToCompleted();
    testInProgressCanMoveToCancelled();
    testPendingCannotMoveDirectlyToCompleted();
    testInProgressCannotReturnToPending();
    testCompletedIsTerminal();
    testCancelledIsTerminal();
    testUnsupportedStatusIsRejected();
    testNonexistentServiceRequestIsHandledSafely();
    testSuccessfulTransitionPreservesServiceRequestInformation();
    testInvalidTransitionDoesNotModifyPersistence();
    testSameStatusRequestIsRejected();
    testMultipleSequentialTransitionsFollowCorrectWorkflow();
}

// ---------------------------------------------------------------------------
// T11 includes
// ---------------------------------------------------------------------------

#include "services/ServiceRequestQueryService.h"

// ---------------------------------------------------------------------------
// T11 Helpers
// ---------------------------------------------------------------------------

// Saves a ServiceRequest directly through the repository.
// Returns the generated id.
static int saveServiceRequest(
    csms::ServiceRequestRepository& repo,
    int residentId,
    const std::string& serviceType,
    const std::string& description,
    const std::string& dateRequested,
    const std::string& status = "Pending")
{
    csms::ServiceRequest r(residentId, serviceType, description,
                           dateRequested, status);
    const csms::ServiceRequest saved = repo.save(r);
    return saved.getId().value();
}

// ---------------------------------------------------------------------------
// T11 Test Helper Functions
// ---------------------------------------------------------------------------

// Test 1 — List all Service Requests returns all persisted records
void testListAllServiceRequestsReturnsAllRecords()
{
    const std::string dbPath = makeTempDbPath("t11_list_all");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Desc 1", "2026-09-01");
        saveServiceRequest(requestRepo, residentId,
                           "Certificate Request", "Desc 2", "2026-09-15");
        saveServiceRequest(requestRepo, residentId,
                           "Community Assistance", "Desc 3", "2026-09-20");

        const std::vector<csms::ServiceRequest> result =
            service.listServiceRequests();

        assert(result.size() == 3u);
    }
}

// Test 2 — Empty database returns empty collection (not null, not exception)
void testListServiceRequestsWhenEmptyReturnsEmptyCollection()
{
    const std::string dbPath = makeTempDbPath("t11_list_empty");

    {
        csms::Database db(dbPath);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const std::vector<csms::ServiceRequest> result =
            service.listServiceRequests();

        assert(result.empty());
    }
}

// Test 3 — List uses required order: date_requested DESC, id DESC
void testListServiceRequestsOrderByDateDescThenIdDesc()
{
    const std::string dbPath = makeTempDbPath("t11_ordering");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        // Insert in an order that differs from the expected display order.
        // IDs are assigned sequentially by SQLite AUTOINCREMENT.
        const int id8  = saveServiceRequest(requestRepo, residentId,
                             "Type A", "Desc A", "2026-09-15");
        const int id12 = saveServiceRequest(requestRepo, residentId,
                             "Type B", "Desc B", "2026-09-18");
        const int id15 = saveServiceRequest(requestRepo, residentId,
                             "Type C", "Desc C", "2026-09-18");
        const int id20 = saveServiceRequest(requestRepo, residentId,
                             "Type D", "Desc D", "2026-09-10");

        const std::vector<csms::ServiceRequest> result =
            service.listServiceRequests();

        assert(result.size() == 4u);

        // Expected: id15 (2026-09-18), id12 (2026-09-18),
        //           id8  (2026-09-15), id20 (2026-09-10)
        assert(result[0].getId().value() == id15);
        assert(result[1].getId().value() == id12);
        assert(result[2].getId().value() == id8);
        assert(result[3].getId().value() == id20);
    }
}

// Test 4 — View one Service Request by ID returns correct record
void testViewServiceRequestByIdReturnsCorrectRecord()
{
    const std::string dbPath = makeTempDbPath("t11_view_by_id");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        const int requestId = saveServiceRequest(
            requestRepo, residentId,
            "Certificate Request",
            "Requesting certificate of residency.",
            "2026-09-20");

        const std::optional<csms::ServiceRequest> result =
            service.viewServiceRequest(requestId);

        assert(result.has_value());
        assert(result->getId().has_value());
        assert(result->getId().value()    == requestId);
        assert(result->getResidentId()    == residentId);
        assert(result->getServiceType()   == "Certificate Request");
        assert(result->getDescription()   == "Requesting certificate of residency.");
        assert(result->getDateRequested() == "2026-09-20");
        assert(result->getStatus()        == "Pending");
    }
}

// Test 5 — View nonexistent Service Request returns nullopt (no crash)
void testViewNonexistentServiceRequestReturnsNullopt()
{
    const std::string dbPath = makeTempDbPath("t11_view_not_found");

    {
        csms::Database db(dbPath);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const std::optional<csms::ServiceRequest> result =
            service.viewServiceRequest(999999);

        assert(!result.has_value());
    }
}

// Test 6 — Search by serviceType partial term (case-insensitive)
void testSearchByServiceTypePartialCaseInsensitive()
{
    const std::string dbPath = makeTempDbPath("t11_search_service_type");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Some description", "2026-09-01");
        saveServiceRequest(requestRepo, residentId,
                           "Certificate Request", "Some description", "2026-09-02");

        // Mixed-case partial match against serviceType.
        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("BARANGAY");

        assert(result.size() == 1u);
        assert(result[0].getServiceType() == "Barangay Clearance");
    }
}

// Test 7 — Search by description partial term (case-insensitive)
void testSearchByDescriptionPartialCaseInsensitive()
{
    const std::string dbPath = makeTempDbPath("t11_search_description");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Type A", "Requesting employment clearance", "2026-09-01");
        saveServiceRequest(requestRepo, residentId,
                           "Type B", "General community help", "2026-09-02");

        // Mixed-case partial match against description.
        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("EMPLOYMENT");

        assert(result.size() == 1u);
        assert(result[0].getDescription() == "Requesting employment clearance");
    }
}

// Test 8 — Blank search term returns all Service Requests
void testBlankSearchTermReturnsAllServiceRequests()
{
    const std::string dbPath = makeTempDbPath("t11_blank_search");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Type A", "Desc A", "2026-09-01");
        saveServiceRequest(requestRepo, residentId,
                           "Type B", "Desc B", "2026-09-02");

        // Whitespace-only term must behave like listServiceRequests().
        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("   ");

        assert(result.size() == 2u);
    }
}

// Test 9 — Filter by status returns only matching records
void testFilterByStatusReturnsMatchingRecordsOnly()
{
    const std::string dbPath = makeTempDbPath("t11_filter_status");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Type A", "Desc A", "2026-09-01", "Pending");
        saveServiceRequest(requestRepo, residentId,
                           "Type B", "Desc B", "2026-09-02", "In Progress");
        saveServiceRequest(requestRepo, residentId,
                           "Type C", "Desc C", "2026-09-03", "Pending");

        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("", "Pending");

        assert(result.size() == 2u);

        for (const auto& sr : result)
        {
            assert(sr.getStatus() == "Pending");
        }
    }
}

// Test 10 — Combine text search and status filter
void testCombineTextSearchAndStatusFilter()
{
    const std::string dbPath = makeTempDbPath("t11_search_and_filter");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Desc A", "2026-09-01", "Pending");
        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Desc B", "2026-09-02", "Completed");
        saveServiceRequest(requestRepo, residentId,
                           "Certificate Request", "Desc C", "2026-09-03", "Pending");

        // Only the Barangay Clearance + Pending combination should match.
        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("Barangay", "Pending");

        assert(result.size() == 1u);
        assert(result[0].getServiceType() == "Barangay Clearance");
        assert(result[0].getStatus()      == "Pending");
    }
}

// Test 11 — Search with no match returns empty collection
void testSearchWithNoMatchReturnsEmptyCollection()
{
    const std::string dbPath = makeTempDbPath("t11_search_no_match");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Desc", "2026-09-01");

        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("ZzzNoMatchTerm");

        assert(result.empty());
    }
}

// Test 12 — Unsupported status filter throws std::invalid_argument
void testUnsupportedStatusFilterThrows()
{
    const std::string dbPath = makeTempDbPath("t11_invalid_status");

    {
        csms::Database db(dbPath);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        bool threw = false;
        try
        {
            service.searchServiceRequests("", "Approved");
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }
}

// Test 13 — Search preserves all Service Request information
void testSearchPreservesAllServiceRequestInformation()
{
    const std::string dbPath = makeTempDbPath("t11_search_info");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        const int requestId = saveServiceRequest(
            requestRepo, residentId,
            "Certificate Request",
            "Requesting certificate of residency.",
            "2026-09-15");

        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("Certificate");

        assert(result.size() == 1u);
        assert(result[0].getId().has_value());
        assert(result[0].getId().value()    == requestId);
        assert(result[0].getResidentId()    == residentId);
        assert(result[0].getServiceType()   == "Certificate Request");
        assert(result[0].getDescription()   == "Requesting certificate of residency.");
        assert(result[0].getDateRequested() == "2026-09-15");
        assert(result[0].getStatus()        == "Pending");
    }
}

// Test 14 — Record matching both serviceType and description appears only once
void testSearchDoesNotDuplicateWhenBothColumnsMatch()
{
    const std::string dbPath = makeTempDbPath("t11_no_duplicate");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService service(requestRepo);

        const int residentId = saveActiveResident(residentRepo);

        // The term "Clearance" appears in both serviceType and description.
        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance",
                           "Clearance for employment requirements.",
                           "2026-09-01");

        const std::vector<csms::ServiceRequest> result =
            service.searchServiceRequests("Clearance");

        // Must appear exactly once despite matching both columns.
        assert(result.size() == 1u);
    }
}

// ---------------------------------------------------------------------------
// T11 Drogon test wrapper — runs all T11 query scenarios
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestQueryTest)
{
    testListAllServiceRequestsReturnsAllRecords();
    testListServiceRequestsWhenEmptyReturnsEmptyCollection();
    testListServiceRequestsOrderByDateDescThenIdDesc();
    testViewServiceRequestByIdReturnsCorrectRecord();
    testViewNonexistentServiceRequestReturnsNullopt();
    testSearchByServiceTypePartialCaseInsensitive();
    testSearchByDescriptionPartialCaseInsensitive();
    testBlankSearchTermReturnsAllServiceRequests();
    testFilterByStatusReturnsMatchingRecordsOnly();
    testCombineTextSearchAndStatusFilter();
    testSearchWithNoMatchReturnsEmptyCollection();
    testUnsupportedStatusFilterThrows();
    testSearchPreservesAllServiceRequestInformation();
    testSearchDoesNotDuplicateWhenBothColumnsMatch();
}

// ---------------------------------------------------------------------------
// T12 — Resident Management Interface (Service-layer integration tests)
//
// T12 tests exercise the service capabilities that the Resident Management
// Interface exposes through its handlers: listing, searching, registering,
// updating, deactivating, and safe handling of missing records.
//
// All tests use makeTempDbPath for isolation and follow the assert()-based
// helper pattern established in T04–T07.
// ---------------------------------------------------------------------------

// Test 1 — List residents returns all persisted residents
void testT12ListResidentsReturnsAll()
{
    const std::string dbPath = makeTempDbPath("t12_list_all");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentQueryService query(repo);

    saveResident(repo, "Ana",   "Reyes");
    saveResident(repo, "Ben",   "Santos");
    saveResident(repo, "Clara", "Tan");

    const auto result = query.listResidents();

    assert(result.size() == 3u);
}

// Test 2 — List residents returns empty collection when none exist
void testT12ListResidentsReturnsEmptyWhenNone()
{
    const std::string dbPath = makeTempDbPath("t12_list_empty");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentQueryService query(repo);

    const auto result = query.listResidents();

    assert(result.empty());
}

// Test 3 — List residents uses T05 ordering (lastName ASC, firstName ASC, id ASC)
void testT12ListResidentsUsesCorrectOrdering()
{
    const std::string dbPath = makeTempDbPath("t12_ordering");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentQueryService query(repo);

    saveResident(repo, "Pedro", "Cruz");
    saveResident(repo, "Ana",   "Santos");
    saveResident(repo, "Juan",  "Cruz");

    const auto result = query.listResidents();

    assert(result.size() == 3u);
    // Cruz/Juan < Cruz/Pedro < Santos/Ana
    assert(result[0].getLastName()  == "Cruz");
    assert(result[0].getFirstName() == "Juan");
    assert(result[1].getLastName()  == "Cruz");
    assert(result[1].getFirstName() == "Pedro");
    assert(result[2].getLastName()  == "Santos");
}

// Test 4 — Search residents by partial first name (case-insensitive)
void testT12SearchByFirstName()
{
    const std::string dbPath = makeTempDbPath("t12_search_first");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentQueryService query(repo);

    saveResident(repo, "Juan",  "Cruz");
    saveResident(repo, "Maria", "Santos");

    const auto result = query.searchResidents("jua");

    assert(result.size() == 1u);
    assert(result[0].getFirstName() == "Juan");
}

// Test 5 — Search residents by partial last name (case-insensitive)
void testT12SearchByLastName()
{
    const std::string dbPath = makeTempDbPath("t12_search_last");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentQueryService query(repo);

    saveResident(repo, "Juan",  "Dela Cruz");
    saveResident(repo, "Maria", "Santos");

    const auto result = query.searchResidents("DELA");

    assert(result.size() == 1u);
    assert(result[0].getLastName() == "Dela Cruz");
}

// Test 6 — Register a valid resident succeeds
void testT12RegisterValidResidentSucceeds()
{
    const std::string dbPath = makeTempDbPath("t12_register");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentRegistrationService service(validator, repo);

    csms::Resident r("Juan", "Dela Cruz",
                     "Barangay Santo Tomas", "09171234567", "juan@example.com");

    const auto result = service.registerResident(r);

    assert(result.success);
    assert(result.resident.has_value());
    assert(result.resident->getId() > 0);
    assert(result.errors.empty());
}

// Test 7 — Register invalid resident returns validation errors
void testT12RegisterInvalidResidentReturnsErrors()
{
    const std::string dbPath = makeTempDbPath("t12_register_invalid");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentRegistrationService service(validator, repo);

    // Empty first name — fails T02
    csms::Resident r("", "Dela Cruz",
                     "Barangay Santo Tomas", "09171234567", "juan@example.com");

    const auto result = service.registerResident(r);

    assert(!result.success);
    assert(!result.errors.empty());
    assert(containsValidationError(result.errors, "firstName"));
    assert(!result.resident.has_value());
}

// Test 8 — Registered resident is visible through listing
void testT12RegisteredResidentAppearsInListing()
{
    const std::string dbPath = makeTempDbPath("t12_register_visible");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentRegistrationService regService(validator, repo);
    csms::ResidentQueryService queryService(repo);

    csms::Resident r("Ana", "Santos",
                     "123 Mabini St", "09171234567", "ana@example.com");

    const auto regResult = regService.registerResident(r);
    assert(regResult.success);

    const auto listed = queryService.listResidents();
    assert(listed.size() == 1u);
    assert(listed[0].getFirstName() == "Ana");
}

// Test 9 — Update resident with valid data succeeds
void testT12UpdateResidentSucceeds()
{
    const std::string dbPath = makeTempDbPath("t12_update");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentUpdateService service(validator, repo);

    csms::Resident original("Juan", "Cruz",
                             "Old Address", "09171234567", "juan@example.com");
    const int id = repo.save(original).getId();

    csms::Resident proposed("Juan Miguel", "Dela Cruz",
                             "New Address", "09181234567", "jm@example.com");

    const auto result = service.updateResident(id, proposed);

    assert(result.success);
    assert(result.resident.has_value());
    assert(result.resident->getFirstName() == "Juan Miguel");
    assert(result.resident->getId()        == id);
}

// Test 10 — Update resident with invalid data returns validation errors
void testT12UpdateResidentInvalidReturnsErrors()
{
    const std::string dbPath = makeTempDbPath("t12_update_invalid");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentUpdateService service(validator, repo);

    csms::Resident original("Juan", "Cruz",
                             "Address", "09171234567", "juan@example.com");
    const int id = repo.save(original).getId();

    // Empty first name — fails T02
    csms::Resident proposed("", "Cruz",
                             "Address", "09171234567", "juan@example.com");

    const auto result = service.updateResident(id, proposed);

    assert(!result.success);
    assert(!result.notFound);
    assert(containsValidationError(result.errors, "firstName"));
}

// Test 11 — Update nonexistent resident sets notFound flag
void testT12UpdateNonexistentResidentReturnsNotFound()
{
    const std::string dbPath = makeTempDbPath("t12_update_not_found");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentValidator validator;
    csms::ResidentUpdateService service(validator, repo);

    csms::Resident proposed("Juan", "Cruz",
                             "Address", "09171234567", "juan@example.com");

    const auto result = service.updateResident(999999, proposed);

    assert(!result.success);
    assert(result.notFound);
}

// Test 12 — Deactivate active resident succeeds and sets status Inactive
void testT12DeactivateActiveResidentSucceeds()
{
    const std::string dbPath = makeTempDbPath("t12_deactivate");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentDeactivationService service(repo);

    csms::Resident r("Juan", "Cruz",
                     "Address", "09171234567", "juan@example.com", "Active");
    const int id = repo.save(r).getId();

    const auto result = service.deactivateResident(id);

    assert(result.success);
    assert(!result.notFound);
    assert(result.resident.has_value());
    assert(result.resident->getStatus() == "Inactive");
}

// Test 13 — Inactive resident remains visible in listing (T05 does not filter by status)
void testT12InactiveResidentRemainsVisible()
{
    const std::string dbPath = makeTempDbPath("t12_inactive_visible");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentDeactivationService deactivationService(repo);
    csms::ResidentQueryService queryService(repo);

    csms::Resident r("Maria", "Santos",
                     "Address", "09171234567", "maria@example.com", "Active");
    const int id = repo.save(r).getId();

    deactivationService.deactivateResident(id);

    const auto listed = queryService.listResidents();
    assert(listed.size() == 1u);
    assert(listed[0].getStatus() == "Inactive");
}

// Test 14 — Deactivate nonexistent resident sets notFound flag safely
void testT12DeactivateNonexistentResidentReturnsNotFound()
{
    const std::string dbPath = makeTempDbPath("t12_deactivate_not_found");

    csms::Database db(dbPath);
    csms::ResidentRepository repo(db);
    csms::ResidentDeactivationService service(repo);

    const auto result = service.deactivateResident(999999);

    assert(!result.success);
    assert(result.notFound);
    assert(!result.resident.has_value());
}

// ---------------------------------------------------------------------------
// T12 Drogon test wrapper — runs all T12 Resident Management Interface tests
// ---------------------------------------------------------------------------

DROGON_TEST(ResidentManagementInterfaceTest)
{
    testT12ListResidentsReturnsAll();
    testT12ListResidentsReturnsEmptyWhenNone();
    testT12ListResidentsUsesCorrectOrdering();
    testT12SearchByFirstName();
    testT12SearchByLastName();
    testT12RegisterValidResidentSucceeds();
    testT12RegisterInvalidResidentReturnsErrors();
    testT12RegisteredResidentAppearsInListing();
    testT12UpdateResidentSucceeds();
    testT12UpdateResidentInvalidReturnsErrors();
    testT12UpdateNonexistentResidentReturnsNotFound();
    testT12DeactivateActiveResidentSucceeds();
    testT12InactiveResidentRemainsVisible();
    testT12DeactivateNonexistentResidentReturnsNotFound();
}

// ---------------------------------------------------------------------------
// T13 — Service Request Management Interface (Service-layer integration tests)
// ---------------------------------------------------------------------------

// Helper — saves an Active Resident for T13 tests and returns its id.
static int saveActiveResidentForT13(csms::ResidentRepository& repo)
{
    csms::Resident r("Test", "User", "Test Address", "09171234567", "test@example.com");
    return repo.save(r).getId();
}

// Helper — saves a ServiceRequest directly and returns the generated id.
static int saveServiceRequestForT13(
    csms::ServiceRequestRepository& repo,
    int residentId,
    const std::string& serviceType,
    const std::string& description,
    const std::string& dateRequested,
    const std::string& status = "Pending")
{
    csms::ServiceRequest sr(residentId, serviceType, description, dateRequested, status);
    csms::ServiceRequest saved = repo.save(sr);
    assert(saved.getId().has_value());
    return saved.getId().value();
}

// Test 1 — Valid Service Request submission succeeds
void testT13SubmitValidRequestSucceeds()
{
    const std::string dbPath = makeTempDbPath("t13_submit_success");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(result.success);
    assert(result.serviceRequest.has_value());
    assert(result.serviceRequest->getId().value() > 0);
}

// Test 2 — Submitted Service Request status is Pending
void testT13SubmitSetsStatusPending()
{
    const std::string dbPath = makeTempDbPath("t13_status_pending");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(result.success);
    assert(result.serviceRequest.has_value());

    const int id = result.serviceRequest->getId().value();
    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "Pending");
}

// Test 3 — Blank serviceType submission fails validation
void testT13SubmitValidationErrorMissingServiceType()
{
    const std::string dbPath = makeTempDbPath("t13_missing_service_type");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "", "Some description", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(!result.success);
    assert(!result.errors.empty());
    assert(containsValidationError(result.errors, "serviceType"));
}

// Test 4 — Blank description submission fails validation
void testT13SubmitValidationErrorMissingDescription()
{
    const std::string dbPath = makeTempDbPath("t13_missing_description");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance", "", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(!result.success);
    assert(!result.errors.empty());
    assert(containsValidationError(result.errors, "description"));
}

// Test 5 — Blank dateRequested submission fails validation
void testT13SubmitValidationErrorMissingDateRequested()
{
    const std::string dbPath = makeTempDbPath("t13_missing_date");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance", "Some description", "");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    assert(!result.success);
    assert(!result.errors.empty());
    assert(containsValidationError(result.errors, "dateRequested"));
}

// Test 6 — Nonexistent Resident prevents submission
void testT13SubmitResidentNotFound()
{
    const std::string dbPath = makeTempDbPath("t13_resident_not_found");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int countBefore = countServiceRequests(dbPath);

    csms::ServiceRequest request(999999, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    const int countAfter = countServiceRequests(dbPath);

    assert(!result.success);
    assert(result.residentNotFound);
    assert(countAfter == countBefore);
}

// Test 7 — Inactive Resident cannot submit
void testT13SubmitInactiveResidentRejected()
{
    const std::string dbPath = makeTempDbPath("t13_inactive_resident");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int inactiveId = saveInactiveResident(residentRepo);

    const int countBefore = countServiceRequests(dbPath);

    csms::ServiceRequest request(inactiveId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult result =
        submissionService.submitRequest(request);

    const int countAfter = countServiceRequests(dbPath);

    assert(!result.success);
    assert(result.residentInactive);
    assert(countAfter == countBefore);
}

// Test 8 — List returns all submitted requests
void testT13ListReturnsAllSubmittedRequests()
{
    const std::string dbPath = makeTempDbPath("t13_list_all");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest r1(residentId, "Type A", "Desc A", "2026-09-01");
    csms::ServiceRequest r2(residentId, "Type B", "Desc B", "2026-09-02");
    csms::ServiceRequest r3(residentId, "Type C", "Desc C", "2026-09-03");

    submissionService.submitRequest(r1);
    submissionService.submitRequest(r2);
    submissionService.submitRequest(r3);

    const auto result = queryService.listServiceRequests();

    assert(result.size() == 3u);
}

// Test 9 — View by ID returns correct request
void testT13ViewByIdReturnsCorrectRequest()
{
    const std::string dbPath = makeTempDbPath("t13_view_by_id");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Certificate Request",
                                 "Requesting certificate.", "2026-09-20");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    assert(submitResult.serviceRequest.has_value());

    const int id = submitResult.serviceRequest->getId().value();

    const std::optional<csms::ServiceRequest> found =
        queryService.viewServiceRequest(id);

    assert(found.has_value());
    assert(found->getId().value()    == id);
    assert(found->getResidentId()    == residentId);
    assert(found->getServiceType()   == "Certificate Request");
    assert(found->getDescription()   == "Requesting certificate.");
    assert(found->getDateRequested() == "2026-09-20");
    assert(found->getStatus()        == "Pending");
}

// Test 10 — View nonexistent request returns nullopt
void testT13ViewNonexistentReturnsNullopt()
{
    const std::string dbPath = makeTempDbPath("t13_view_not_found");

    csms::Database db(dbPath);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const std::optional<csms::ServiceRequest> result =
        queryService.viewServiceRequest(999999);

    assert(!result.has_value());
}

// Test 11 — Pending can start processing (move to In Progress)
void testT13ManageStatusPendingToInProgress()
{
    const std::string dbPath = makeTempDbPath("t13_pending_to_inprogress");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    const int id = submitResult.serviceRequest->getId().value();

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(id, "In Progress");

    assert(result.success);

    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "In Progress");
}

// Test 12 — In Progress can be completed
void testT13ManageStatusInProgressToCompleted()
{
    const std::string dbPath = makeTempDbPath("t13_inprogress_to_completed");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    const int id = submitResult.serviceRequest->getId().value();

    statusService.manageStatus(id, "In Progress");

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(id, "Completed");

    assert(result.success);

    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "Completed");
}

// Test 13 — Pending can be cancelled
void testT13ManageStatusPendingToCancelled()
{
    const std::string dbPath = makeTempDbPath("t13_pending_to_cancelled");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    const int id = submitResult.serviceRequest->getId().value();

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(id, "Cancelled");

    assert(result.success);

    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "Cancelled");
}

// Test 14 — Completed is terminal
void testT13ManageStatusCompletedIsTerminal()
{
    const std::string dbPath = makeTempDbPath("t13_completed_terminal");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    const int id = submitResult.serviceRequest->getId().value();

    statusService.manageStatus(id, "In Progress");
    statusService.manageStatus(id, "Completed");

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(id, "Cancelled");

    assert(!result.success);
    assert(result.invalidTransition);

    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "Completed");
}

// Test 15 — Nonexistent request returns notFound
void testT13ManageStatusNotFoundReturnsNotFound()
{
    const std::string dbPath = makeTempDbPath("t13_status_not_found");

    csms::Database db(dbPath);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(999999, "In Progress");

    assert(!result.success);
    assert(result.notFound);
}

// Test 16 — Invalid direct transition is rejected
void testT13InvalidDirectTransitionIsStillRejected()
{
    const std::string dbPath = makeTempDbPath("t13_invalid_transition");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);
    csms::ServiceRequestStatusService statusService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    const csms::ServiceRequestSubmissionResult submitResult =
        submissionService.submitRequest(request);

    assert(submitResult.success);
    const int id = submitResult.serviceRequest->getId().value();

    const csms::ServiceRequestStatusResult result =
        statusService.manageStatus(id, "Completed");

    assert(result.invalidTransition == true);

    const std::optional<csms::ServiceRequest> stored = requestRepo.findById(id);
    assert(stored.has_value());
    assert(stored->getStatus() == "Pending");
}

// Test 17 — List uses required ordering (dateRequested DESC)
void testT13RequiredOrderingIsPreserved()
{
    const std::string dbPath = makeTempDbPath("t13_ordering");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    saveServiceRequestForT13(requestRepo, residentId, "Type A", "Desc A", "2026-09-10");
    saveServiceRequestForT13(requestRepo, residentId, "Type B", "Desc B", "2026-09-20");
    saveServiceRequestForT13(requestRepo, residentId, "Type C", "Desc C", "2026-09-15");

    const std::vector<csms::ServiceRequest> result = queryService.listServiceRequests();

    assert(result.size() == 3u);
    assert(result[0].getDateRequested() == "2026-09-20");
    assert(result[1].getDateRequested() == "2026-09-15");
    assert(result[2].getDateRequested() == "2026-09-10");
}

// Test 18 — Text search uses T11 behavior (case-insensitive partial)
void testT13TextSearchUsesTT11Behavior()
{
    const std::string dbPath = makeTempDbPath("t13_text_search");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    saveServiceRequestForT13(requestRepo, residentId, "Barangay Clearance", "Desc A", "2026-09-01");
    saveServiceRequestForT13(requestRepo, residentId, "Certificate Request", "Desc B", "2026-09-02");

    const std::vector<csms::ServiceRequest> result =
        queryService.searchServiceRequests("cLeAr", "");

    assert(result.size() == 1u);
    assert(result[0].getServiceType() == "Barangay Clearance");
}

// Test 19 — Status filter uses T11 behavior
void testT13StatusFilterUsesT11Behavior()
{
    const std::string dbPath = makeTempDbPath("t13_status_filter");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestQueryService queryService(requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    saveServiceRequestForT13(requestRepo, residentId, "Type A", "Desc A", "2026-09-01", "Pending");
    saveServiceRequestForT13(requestRepo, residentId, "Type B", "Desc B", "2026-09-02", "Completed");

    const std::vector<csms::ServiceRequest> result =
        queryService.searchServiceRequests("", "Pending");

    assert(result.size() == 1u);
    assert(result[0].getStatus() == "Pending");
}

// Test 20 — Submission does not modify the Resident
void testT13SubmitDoesNotModifyResident()
{
    const std::string dbPath = makeTempDbPath("t13_resident_unchanged");

    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository requestRepo(db);
    csms::ServiceRequestValidator validator;
    csms::ServiceRequestSubmissionService submissionService(validator, residentRepo, requestRepo);

    const int residentId = saveActiveResidentForT13(residentRepo);

    // Capture before
    const std::optional<csms::Resident> before = residentRepo.findById(residentId);
    assert(before.has_value());

    csms::ServiceRequest request(residentId, "Barangay Clearance",
                                 "Requesting clearance.", "2026-09-25");

    submissionService.submitRequest(request);

    // Capture after
    const std::optional<csms::Resident> after = residentRepo.findById(residentId);
    assert(after.has_value());

    assert(after->getId()            == before->getId());
    assert(after->getFirstName()     == before->getFirstName());
    assert(after->getLastName()      == before->getLastName());
    assert(after->getAddress()       == before->getAddress());
    assert(after->getContactNumber() == before->getContactNumber());
    assert(after->getEmail()         == before->getEmail());
    assert(after->getStatus()        == before->getStatus());
}

// ---------------------------------------------------------------------------
// T13 Drogon test wrapper — runs all T13 Service Request Management tests
// ---------------------------------------------------------------------------

DROGON_TEST(ServiceRequestManagementInterfaceTest)
{
    testT13SubmitValidRequestSucceeds();
    testT13SubmitSetsStatusPending();
    testT13SubmitValidationErrorMissingServiceType();
    testT13SubmitValidationErrorMissingDescription();
    testT13SubmitValidationErrorMissingDateRequested();
    testT13SubmitResidentNotFound();
    testT13SubmitInactiveResidentRejected();
    testT13ListReturnsAllSubmittedRequests();
    testT13ViewByIdReturnsCorrectRequest();
    testT13ViewNonexistentReturnsNullopt();
    testT13ManageStatusPendingToInProgress();
    testT13ManageStatusInProgressToCompleted();
    testT13ManageStatusPendingToCancelled();
    testT13ManageStatusCompletedIsTerminal();
    testT13ManageStatusNotFoundReturnsNotFound();
    testT13InvalidDirectTransitionIsStillRejected();
    testT13RequiredOrderingIsPreserved();
    testT13TextSearchUsesTT11Behavior();
    testT13StatusFilterUsesT11Behavior();
    testT13SubmitDoesNotModifyResident();
}

// ---------------------------------------------------------------------------
// T14 includes
// ---------------------------------------------------------------------------

#include "services/DashboardService.h"

// ---------------------------------------------------------------------------
// T14 Helpers
// ---------------------------------------------------------------------------

static int saveResidentForT14(
    csms::ResidentRepository& repo,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& status = "Active")
{
    csms::Resident r(firstName, lastName,
                     "Address", "09171234567",
                     firstName + "." + lastName + "@example.com",
                     status);
    return repo.save(r).getId();
}

static int saveServiceRequestForT14(
    csms::ServiceRequestRepository& repo,
    int residentId,
    const std::string& serviceType,
    const std::string& description,
    const std::string& dateRequested,
    const std::string& status = "Pending")
{
    csms::ServiceRequest r(residentId, serviceType, description, dateRequested, status);
    return repo.save(r).getId().value();
}

// ---------------------------------------------------------------------------
// T14 Test Helper Functions (15 tests)
// ---------------------------------------------------------------------------

// Test 1 — getDashboardData() returns without throwing on empty DB
void testDashboardLoadsSuccessfully()
{
    const std::string dbPath = makeTempDbPath("t14_loads");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    bool threw = false;
    try { svc.getDashboardData(); }
    catch (...) { threw = true; }
    assert(!threw);
}

// Test 2 — empty DB → all counts == 0, recentRequests.empty()
void testEmptyDashboardDisplaysZeroCounts()
{
    const std::string dbPath = makeTempDbPath("t14_empty");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.totalResidents        == 0);
    assert(data.activeResidents       == 0);
    assert(data.inactiveResidents     == 0);
    assert(data.totalServiceRequests  == 0);
    assert(data.pendingRequests       == 0);
    assert(data.inProgressRequests    == 0);
    assert(data.completedRequests     == 0);
    assert(data.cancelledRequests     == 0);
    assert(data.openRequests          == 0);
    assert(data.recentRequests.empty());
}

// Test 3 — persist 3 residents → totalResidents == 3
void testTotalResidentCountIsCorrect()
{
    const std::string dbPath = makeTempDbPath("t14_total_residents");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    saveResidentForT14(residentRepo, "Ana",   "Reyes");
    saveResidentForT14(residentRepo, "Ben",   "Santos");
    saveResidentForT14(residentRepo, "Clara", "Tan");

    const csms::DashboardData data = svc.getDashboardData();
    assert(data.totalResidents == 3);
}

// Test 4 — 2 Active + 1 Inactive → correct status counts and total == active + inactive
void testResidentStatusCountsAreCorrect()
{
    const std::string dbPath = makeTempDbPath("t14_resident_status_counts");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    saveResidentForT14(residentRepo, "Ana",  "Reyes",  "Active");
    saveResidentForT14(residentRepo, "Ben",  "Santos", "Active");
    saveResidentForT14(residentRepo, "Clara", "Tan",   "Inactive");

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.activeResidents   == 2);
    assert(data.inactiveResidents == 1);
    assert(data.totalResidents    == 3);
    assert(data.totalResidents    == data.activeResidents + data.inactiveResidents);
}

// Test 5 — persist 4 service requests → totalServiceRequests == 4
void testTotalServiceRequestCountIsCorrect()
{
    const std::string dbPath = makeTempDbPath("t14_total_sr");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    saveServiceRequestForT14(srRepo, residentId, "Type A", "Desc A", "2026-09-01");
    saveServiceRequestForT14(srRepo, residentId, "Type B", "Desc B", "2026-09-02");
    saveServiceRequestForT14(srRepo, residentId, "Type C", "Desc C", "2026-09-03");
    saveServiceRequestForT14(srRepo, residentId, "Type D", "Desc D", "2026-09-04");

    const csms::DashboardData data = svc.getDashboardData();
    assert(data.totalServiceRequests == 4);
}

// Test 6 — 1 of each status → each count == 1, total == 4
void testServiceRequestStatusCountsAreCorrect()
{
    const std::string dbPath = makeTempDbPath("t14_sr_status_counts");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    saveServiceRequestForT14(srRepo, residentId, "Type A", "Desc A", "2026-09-01", "Pending");
    saveServiceRequestForT14(srRepo, residentId, "Type B", "Desc B", "2026-09-02", "In Progress");
    saveServiceRequestForT14(srRepo, residentId, "Type C", "Desc C", "2026-09-03", "Completed");
    saveServiceRequestForT14(srRepo, residentId, "Type D", "Desc D", "2026-09-04", "Cancelled");

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.pendingRequests    == 1);
    assert(data.inProgressRequests == 1);
    assert(data.completedRequests  == 1);
    assert(data.cancelledRequests  == 1);
    assert(data.totalServiceRequests == 4);
}

// Test 7 — total == pending + inProgress + completed + cancelled
void testServiceRequestCountsReconcile()
{
    const std::string dbPath = makeTempDbPath("t14_sr_reconcile");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    saveServiceRequestForT14(srRepo, residentId, "T A", "D A", "2026-09-01", "Pending");
    saveServiceRequestForT14(srRepo, residentId, "T B", "D B", "2026-09-02", "Pending");
    saveServiceRequestForT14(srRepo, residentId, "T C", "D C", "2026-09-03", "In Progress");
    saveServiceRequestForT14(srRepo, residentId, "T D", "D D", "2026-09-04", "Completed");
    saveServiceRequestForT14(srRepo, residentId, "T E", "D E", "2026-09-05", "Cancelled");

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.totalServiceRequests ==
           data.pendingRequests + data.inProgressRequests +
           data.completedRequests + data.cancelledRequests);
}

// Test 8 — openRequests == pending + inProgress (not counting Completed/Cancelled)
void testOpenRequestCountIsCorrect()
{
    const std::string dbPath = makeTempDbPath("t14_open_count");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    saveServiceRequestForT14(srRepo, residentId, "T A", "D A", "2026-09-01", "Pending");
    saveServiceRequestForT14(srRepo, residentId, "T B", "D B", "2026-09-02", "In Progress");
    saveServiceRequestForT14(srRepo, residentId, "T C", "D C", "2026-09-03", "Completed");
    saveServiceRequestForT14(srRepo, residentId, "T D", "D D", "2026-09-04", "Cancelled");

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.openRequests == data.pendingRequests + data.inProgressRequests);
    assert(data.openRequests == 2);
}

// Test 9 — deactivate Active resident → active-1, inactive+1, total unchanged
void testResidentDeactivationIsReflected()
{
    const std::string dbPath = makeTempDbPath("t14_resident_deactivation");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    saveResidentForT14(residentRepo, "Ana",  "Reyes",  "Active");
    const int id2 = saveResidentForT14(residentRepo, "Ben", "Santos", "Active");
    saveResidentForT14(residentRepo, "Clara", "Tan",   "Active");

    const csms::DashboardData before = svc.getDashboardData();

    residentRepo.deactivateById(id2);

    const csms::DashboardData after = svc.getDashboardData();

    assert(after.totalResidents    == before.totalResidents);
    assert(after.activeResidents   == before.activeResidents - 1);
    assert(after.inactiveResidents == before.inactiveResidents + 1);
}

// Test 10 — updateStatus Pending→InProgress → pending-1, inProgress+1, total unchanged
void testServiceRequestTransitionIsReflected()
{
    const std::string dbPath = makeTempDbPath("t14_sr_transition");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    const int srId = saveServiceRequestForT14(srRepo, residentId,
                                              "Type A", "Desc A",
                                              "2026-09-01", "Pending");

    const csms::DashboardData before = svc.getDashboardData();

    srRepo.updateStatus(srId, "In Progress");

    const csms::DashboardData after = svc.getDashboardData();

    assert(after.totalServiceRequests == before.totalServiceRequests);
    assert(after.pendingRequests      == before.pendingRequests - 1);
    assert(after.inProgressRequests   == before.inProgressRequests + 1);
}

// Test 11 — In Progress → Completed → openRequests decreases, total unchanged
void testTerminalTransitionUpdatesOpenCount()
{
    const std::string dbPath = makeTempDbPath("t14_terminal_transition");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    const int srId = saveServiceRequestForT14(srRepo, residentId,
                                              "Type A", "Desc A",
                                              "2026-09-01", "In Progress");

    const csms::DashboardData before = svc.getDashboardData();

    srRepo.updateStatus(srId, "Completed");

    const csms::DashboardData after = svc.getDashboardData();

    assert(after.totalServiceRequests == before.totalServiceRequests);
    assert(after.openRequests         <  before.openRequests);
    assert(after.completedRequests    == before.completedRequests + 1);
}

// Test 12 — persist 7 SRs → recentRequests.size() <= 5
void testRecentRequestsReturnNoMoreThanFive()
{
    const std::string dbPath = makeTempDbPath("t14_recent_limit");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    for (int i = 1; i <= 7; ++i)
    {
        saveServiceRequestForT14(srRepo, residentId,
                                 "Type " + std::to_string(i),
                                 "Desc " + std::to_string(i),
                                 "2026-09-0" + std::to_string(i));
    }

    const csms::DashboardData data = svc.getDashboardData();
    assert(data.recentRequests.size() <= 5u);
}

// Test 13 — recentRequests ordered date_requested DESC then id DESC
void testRecentRequestsUseCorrectOrdering()
{
    const std::string dbPath = makeTempDbPath("t14_recent_ordering");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    // Different dates — newer date must come first
    saveServiceRequestForT14(srRepo, residentId, "Old",    "Desc", "2026-09-01");
    saveServiceRequestForT14(srRepo, residentId, "Middle", "Desc", "2026-09-15");
    saveServiceRequestForT14(srRepo, residentId, "Newest", "Desc", "2026-09-20");

    const csms::DashboardData data = svc.getDashboardData();

    assert(data.recentRequests.size() == 3u);
    assert(data.recentRequests[0].getDateRequested() == "2026-09-20");
    assert(data.recentRequests[1].getDateRequested() == "2026-09-15");
    assert(data.recentRequests[2].getDateRequested() == "2026-09-01");
}

// Test 14 — a Completed SR may appear in recentRequests
void testTerminalRequestsMayAppearInRecentResults()
{
    const std::string dbPath = makeTempDbPath("t14_recent_terminal");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    const int completedId = saveServiceRequestForT14(
        srRepo, residentId,
        "Completed Type", "Desc", "2026-09-20", "Completed");

    const csms::DashboardData data = svc.getDashboardData();

    assert(!data.recentRequests.empty());
    assert(data.recentRequests[0].getId().has_value());
    assert(data.recentRequests[0].getId().value() == completedId);
    assert(data.recentRequests[0].getStatus()     == "Completed");
}

// Test 15 — getDashboardData() is read-only — calling it does not change counts
void testDashboardGetDoesNotModifyPersistence()
{
    const std::string dbPath = makeTempDbPath("t14_readonly");
    csms::Database db(dbPath);
    csms::ResidentRepository residentRepo(db);
    csms::ServiceRequestRepository srRepo(db);
    csms::DashboardService svc(residentRepo, srRepo);

    const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
    saveServiceRequestForT14(srRepo, residentId, "Type A", "Desc A", "2026-09-01");

    const csms::DashboardData before = svc.getDashboardData();
    svc.getDashboardData(); // call again — must not modify anything
    const csms::DashboardData after = svc.getDashboardData();

    assert(after.totalResidents       == before.totalResidents);
    assert(after.totalServiceRequests == before.totalServiceRequests);
    assert(after.pendingRequests      == before.pendingRequests);
    assert(after.activeResidents      == before.activeResidents);
    assert(after.recentRequests.size() == before.recentRequests.size());
}

// ---------------------------------------------------------------------------
// T14 Drogon test wrapper
// ---------------------------------------------------------------------------

DROGON_TEST(DashboardTest)
{
    testDashboardLoadsSuccessfully();
    testEmptyDashboardDisplaysZeroCounts();
    testTotalResidentCountIsCorrect();
    testResidentStatusCountsAreCorrect();
    testTotalServiceRequestCountIsCorrect();
    testServiceRequestStatusCountsAreCorrect();
    testServiceRequestCountsReconcile();
    testOpenRequestCountIsCorrect();
    testResidentDeactivationIsReflected();
    testServiceRequestTransitionIsReflected();
    testTerminalTransitionUpdatesOpenCount();
    testRecentRequestsReturnNoMoreThanFive();
    testRecentRequestsUseCorrectOrdering();
    testTerminalRequestsMayAppearInRecentResults();
    testDashboardGetDoesNotModifyPersistence();
}

// ---------------------------------------------------------------------------
// T15 — System Hardening Tests
//
// Five regression/hardening tests exercising cross-feature invariants and
// persistence-safety properties. Each uses its own isolated temp DB via
// makeTempDbPath(). They reuse helpers already defined earlier in this file
// (saveResidentForT14, saveServiceRequest, saveActiveResident,
//  makeValidServiceRequest, submitNewRequest).
// ---------------------------------------------------------------------------

// Test 1 (Category 1 - Invalid/Malformed Input):
// An unsupported status filter must throw std::invalid_argument and must not
// create or modify any Service Request.
void testUnsupportedServiceRequestStatusSearchReturnsEmptyNotCrash()
{
    const std::string dbPath = makeTempDbPath("t15_unsupported_status");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestQueryService queryService(requestRepo);

        // Persist a few Service Requests for an Active Resident.
        const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");
        saveServiceRequest(requestRepo, residentId,
                           "Barangay Clearance", "Desc 1", "2026-09-01");
        saveServiceRequest(requestRepo, residentId,
                           "Certificate Request", "Desc 2", "2026-09-02");

        const int countBefore = requestRepo.countAll();

        // 'Approved' is not a valid status — must throw std::invalid_argument.
        bool threw = false;
        try
        {
            queryService.searchServiceRequests("clear", "Approved");
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }
        assert(threw);

        // No Service Requests were created or modified (count unchanged).
        const int countAfter = requestRepo.countAll();
        assert(countAfter == countBefore);
    }
}

// Test 2 (Category 2 - Rejected Mutation Protects Persistence):
// A rejected status transition must leave every field of the record identical.
void testInvalidServiceRequestTransitionLeavesRecordUnchanged()
{
    const std::string dbPath = makeTempDbPath("t15_invalid_transition");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestStatusService statusService(requestRepo);

        // Persist a Pending Service Request.
        const int requestId = submitNewRequest(residentRepo, requestRepo);

        // Record the current state via findById.
        const std::optional<csms::ServiceRequest> before =
            requestRepo.findById(requestId);
        assert(before.has_value());

        // Attempt the invalid transition: Pending -> Completed (not allowed).
        const csms::ServiceRequestStatusResult result =
            statusService.manageStatus(requestId, "Completed");
        assert(result.invalidTransition == true);
        assert(!result.success);

        // Retrieve the record again via findById.
        const std::optional<csms::ServiceRequest> after =
            requestRepo.findById(requestId);
        assert(after.has_value());

        // Verify ALL fields are identical to the original.
        assert(after->getId().has_value());
        assert(after->getId().value() == before->getId().value());
        assert(after->getResidentId()    == before->getResidentId());
        assert(after->getServiceType()   == before->getServiceType());
        assert(after->getDescription()   == before->getDescription());
        assert(after->getDateRequested() == before->getDateRequested());
        assert(after->getStatus()        == before->getStatus());
        assert(after->getStatus()        == "Pending");
    }
}

// Test 3 (Category 3 - Read-Only Request Safety):
// Repeated dashboard reads must not mutate persistence; counts read directly
// from the repositories must match the first dashboard call, and no new
// records may appear.
void testRepeatedDashboardLoadsDoNotModifyPersistence()
{
    const std::string dbPath = makeTempDbPath("t15_dashboard_readonly");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::DashboardService dashboardService(residentRepo, requestRepo);

        // Persist 2 Active Residents.
        const int residentId1 = saveResidentForT14(residentRepo, "Ana", "Reyes");
        const int residentId2 = saveResidentForT14(residentRepo, "Ben", "Santos");

        // Persist 3 Service Requests with different statuses.
        saveServiceRequest(requestRepo, residentId1,
                           "Type A", "Desc A", "2026-09-01", "Pending");
        saveServiceRequest(requestRepo, residentId1,
                           "Type B", "Desc B", "2026-09-02", "In Progress");
        saveServiceRequest(requestRepo, residentId1,
                           "Type C", "Desc C", "2026-09-03", "Completed");

        // Call getDashboardData() and record all counts (call 1 of 4).
        const csms::DashboardData first = dashboardService.getDashboardData();

        // Call it 3 more times.
        dashboardService.getDashboardData();
        dashboardService.getDashboardData();
        dashboardService.getDashboardData();

        // Re-read all counts via the repository directly.
        const int residentCountAll = residentRepo.countAll();
        const int residentActive   = residentRepo.countByStatus("Active");
        const int residentInactive = residentRepo.countByStatus("Inactive");
        const int srCountAll       = requestRepo.countAll();
        const int srPending        = requestRepo.countByStatus("Pending");
        const int srInProgress     = requestRepo.countByStatus("In Progress");
        const int srCompleted       = requestRepo.countByStatus("Completed");
        const int srCancelled       = requestRepo.countByStatus("Cancelled");

        // Verify all counts are identical to the first dashboard call.
        assert(residentCountAll == first.totalResidents);
        assert(residentActive   == first.activeResidents);
        assert(residentInactive == first.inactiveResidents);
        assert(srCountAll       == first.totalServiceRequests);
        assert(srPending        == first.pendingRequests);
        assert(srInProgress     == first.inProgressRequests);
        assert(srCompleted      == first.completedRequests);
        assert(srCancelled       == first.cancelledRequests);

        // Verify no new records were created.
        assert(residentCountAll == 2);
        assert(srCountAll       == 3);

        // residentId2 is intentionally persisted but unused beyond counting;
        // reference it so compilers do not warn about an unused variable.
        (void)residentId2;
    }
}

// Test 4 (Category 4 - Cross-Feature Regression):
// Deactivating a resident blocks future Service Request submission (T07+T09)
// and is reflected in the dashboard counts (T14).
void testResidentDeactivationBlocksFutureServiceRequestSubmissionAndUpdatesDashboard()
{
    const std::string dbPath = makeTempDbPath("t15_cross_feature");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ServiceRequestRepository requestRepo(db);
        csms::ServiceRequestValidator validator;
        csms::ServiceRequestSubmissionService submissionService(
            validator, residentRepo, requestRepo);
        csms::ResidentDeactivationService deactivationService(residentRepo);
        csms::DashboardService dashboardService(residentRepo, requestRepo);

        // Persist an Active Resident.
        const int residentId = saveResidentForT14(residentRepo, "Juan", "Cruz");

        // Submit a valid Service Request for that Resident (succeeds).
        const csms::ServiceRequestSubmissionResult result1 =
            submissionService.submitRequest(makeValidServiceRequest(residentId));
        assert(result1.success);

        // Record dashboard counts before deactivation.
        const csms::DashboardData before = dashboardService.getDashboardData();
        assert(before.activeResidents   == 1);
        assert(before.inactiveResidents == 0);
        assert(before.totalResidents    == 1);

        // Deactivate the Resident.
        const csms::ResidentDeactivationResult deactResult =
            deactivationService.deactivateResident(residentId);
        assert(deactResult.success);

        // Record Service Request count before the second attempt.
        const int srCountBefore = requestRepo.countAll();

        // Attempt to submit another Service Request for the now-Inactive
        // Resident — must fail with residentInactive (the task wording
        // "residentNotActive" maps to the actual field residentInactive).
        const csms::ServiceRequestSubmissionResult result2 =
            submissionService.submitRequest(makeValidServiceRequest(residentId));
        assert(!result2.success);
        assert(result2.residentInactive);
        assert(!result2.serviceRequest.has_value());

        // No new Service Request was persisted (count unchanged).
        const int srCountAfter = requestRepo.countAll();
        assert(srCountAfter == srCountBefore);

        // Dashboard reflects the deactivation.
        const csms::DashboardData after = dashboardService.getDashboardData();
        assert(after.totalResidents    == before.totalResidents);       // unchanged
        assert(after.activeResidents   == before.activeResidents - 1);  // decreased by 1
        assert(after.inactiveResidents == before.inactiveResidents + 1); // increased by 1
    }
}

// Test 5 (Category 5 - Repeatability/Boundary):
// Deactivating an already-Inactive resident is safe and idempotent, and
// corrupts no field.
void testRepeatedDeactivationIsSafeAndIdempotent()
{
    const std::string dbPath = makeTempDbPath("t15_idempotent_deactivation");

    {
        csms::Database db(dbPath);
        csms::ResidentRepository residentRepo(db);
        csms::ResidentDeactivationService deactivationService(residentRepo);

        // Persist an Active Resident.
        csms::Resident original(
            "Juan", "Dela Cruz", "Barangay Santo Tomas",
            "09171234567", "juan@example.com", "Active");
        const int id = residentRepo.save(original).getId();

        // Deactivate (verify success).
        const csms::ResidentDeactivationResult result1 =
            deactivationService.deactivateResident(id);
        assert(result1.success);
        assert(!result1.alreadyInactive);
        assert(result1.resident.has_value());
        assert(result1.resident->getStatus() == "Inactive");

        // Deactivate the SAME Resident again (idempotent — still succeeds).
        const csms::ResidentDeactivationResult result2 =
            deactivationService.deactivateResident(id);
        assert(result2.success);
        assert(result2.alreadyInactive);
        assert(result2.resident.has_value());
        assert(result2.resident->getStatus() == "Inactive");

        // Retrieve via findById and verify status is still Inactive (not corrupted).
        const std::optional<csms::Resident> after = residentRepo.findById(id);
        assert(after.has_value());
        assert(after->getStatus() == "Inactive");

        // Verify all other fields are unchanged.
        assert(after->getId()            == id);
        assert(after->getFirstName()     == "Juan");
        assert(after->getLastName()      == "Dela Cruz");
        assert(after->getAddress()       == "Barangay Santo Tomas");
        assert(after->getContactNumber() == "09171234567");
        assert(after->getEmail()         == "juan@example.com");

        // No extra rows were created.
        assert(residentRepo.countAll() == 1);
    }
}

DROGON_TEST(SystemHardeningTest)
{
    testUnsupportedServiceRequestStatusSearchReturnsEmptyNotCrash();
    testInvalidServiceRequestTransitionLeavesRecordUnchanged();
    testRepeatedDashboardLoadsDoNotModifyPersistence();
    testResidentDeactivationBlocksFutureServiceRequestSubmissionAndUpdatesDashboard();
    testRepeatedDeactivationIsSafeAndIdempotent();
}
