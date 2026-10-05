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
