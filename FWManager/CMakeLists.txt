project(FWManager)

add_executable(FWManager 
    src/main.cpp
    src/CertificateHandler.cpp)


find_package(OpenSSL REQUIRED)

# Link with libraries
target_link_libraries(FWManager PRIVATE 
    DemoLib
    OpenSSL::SSL
    OpenSSL::Crypto
)

#
# Tests
#
add_executable(FWManagerTest
    src/CertificateHandler.cpp
    test/CertificateHandlerTest.cpp
    )

add_compile_definitions(TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/data")


# Link with libraries
target_link_libraries(FWManagerTest PRIVATE Catch2::Catch2WithMain OpenSSL::Crypto OpenSSL::SSL)

# for coverage: ensure tests are executed in debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(FWManagerTest PRIVATE --coverage)
    target_link_options(FWManagerTest PRIVATE --coverage)
endif()

target_include_directories(FWManagerTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

add_test(NAME FWManagerTest COMMAND FWManagerTest)

# Custom target to run tests and generate coverage report 
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_target(coverageFWManager 
    COMMAND ${CMAKE_CTEST_COMMAND} -T test --output-on-failure 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --xml -o coverage.xml 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --html --html-details -o coverage.html
    DEPENDS FWManagerTest )
endif()
