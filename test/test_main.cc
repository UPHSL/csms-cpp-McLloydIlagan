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
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    f1.get();
    int status = test::run(argc, argv);

    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
