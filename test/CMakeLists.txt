cmake_minimum_required(VERSION 3.5)

enable_testing()

add_executable(csms_cpp_test
    test_main.cc
    ../models/ResidentValidator.cc
    ../models/ServiceRequestValidator.cc
    ../database/Database.cc
    ../repositories/ResidentRepository.cc
    ../repositories/ServiceRequestRepository.cc
    ../services/ResidentRegistrationService.cc
    ../services/ResidentQueryService.cc
    ../services/ResidentUpdateService.cc
    ../services/ResidentDeactivationService.cc
    ../services/ServiceRequestSubmissionService.cc
    ../services/ServiceRequestStatusService.cc
)

target_include_directories(csms_cpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(csms_cpp_test PRIVATE Drogon::Drogon)

# SQLite3 is already found by the parent CMakeLists.txt via find_package(SQLite3 REQUIRED).
# We link using the imported target directly — no second find_package call needed.
target_link_libraries(csms_cpp_test PRIVATE SQLite3_lib)

ParseAndAddDrogonTests(csms_cpp_test)

# Set a per-test timeout so CTest does not hang waiting for the Drogon
# event loop to fully release its socket between consecutive test runs.
# Each individual test completes well within 10 seconds on this machine.
set_tests_properties(
    BasicTest
    ResidentCreationTest
    ResidentInformationAccessTest
    ResidentStatusTest
    ResidentValidationTest
    ResidentPersistenceTest
    ResidentRegistrationTest
    ResidentSearchListTest
    ResidentUpdateTest
    ResidentDeactivationTest
    ServiceRequestDomainTest
    ServiceRequestValidationTest
    ServiceRequestSubmissionTest
    ServiceRequestStatusTest
    PROPERTIES TIMEOUT 10
)
