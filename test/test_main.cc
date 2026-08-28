#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>

#include "models/Resident.h"
#include "models/ResidentValidator.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

bool containsValidationError(
    const std::vector<std::string>& errors,
    const std::string& field)
{
    return std::find(errors.begin(), errors.end(), field) != errors.end();
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
