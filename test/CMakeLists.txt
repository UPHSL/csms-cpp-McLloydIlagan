cmake_minimum_required(VERSION 3.5)

enable_testing()

add_executable(csms_cpp_test
    test_main.cc
    ../models/ResidentValidator.cc
    ../database/Database.cc
    ../repositories/ResidentRepository.cc
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
