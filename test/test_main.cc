#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "models/Resident.h"

DROGON_TEST(BasicTest)
{
    // Existing placeholder test — must continue to pass
}

// Test 1: Resident Creation
// Verify that a Resident can be created using valid Resident information.
DROGON_TEST(ResidentCreationTest)
{
    csms::Resident resident(
        1,
        "Juan",
        "dela Cruz",
        "123 Mabini St, Manila",
        "09171234567",
        "juan.delacruz@email.com",
        csms::ResidentStatus::Active
    );

    CHECK(resident.getId() == 1);
    CHECK(resident.getFirstName() == "Juan");
    CHECK(resident.getLastName() == "dela Cruz");
}

// Test 2: Resident Information Access
// Verify that Resident information can be assigned and retrieved correctly.
DROGON_TEST(ResidentInformationAccessTest)
{
    csms::Resident resident;

    resident.setId(2);
    resident.setFirstName("Maria");
    resident.setLastName("Santos");
    resident.setAddress("456 Rizal Ave, Quezon City");
    resident.setContactNumber("09281234567");
    resident.setEmail("maria.santos@email.com");
    resident.setStatus(csms::ResidentStatus::Active);

    CHECK(resident.getId() == 2);
    CHECK(resident.getFirstName() == "Maria");
    CHECK(resident.getLastName() == "Santos");
    CHECK(resident.getAddress() == "456 Rizal Ave, Quezon City");
    CHECK(resident.getContactNumber() == "09281234567");
    CHECK(resident.getEmail() == "maria.santos@email.com");
    CHECK(resident.getStatus() == csms::ResidentStatus::Active);
}

// Test 3: Resident Status
// Verify that the Resident model can represent the Active status.
DROGON_TEST(ResidentStatusTest)
{
    csms::Resident resident(
        3,
        "Pedro",
        "Reyes",
        "789 Bonifacio St, Pasig",
        "09391234567",
        "pedro.reyes@email.com",
        csms::ResidentStatus::Active
    );

    CHECK(resident.getStatus() == csms::ResidentStatus::Active);
}

int main(int argc, char** argv) 
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
