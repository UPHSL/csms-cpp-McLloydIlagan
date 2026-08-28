cmake_minimum_required(VERSION 3.5)

enable_testing()

add_executable(csms_cpp_test test_main.cc ../models/ResidentValidator.cc)

target_include_directories(csms_cpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(csms_cpp_test PRIVATE Drogon::Drogon)

ParseAndAddDrogonTests(csms_cpp_test)
