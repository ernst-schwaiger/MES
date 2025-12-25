project(FWMManager)

add_executable(FWMManager 
    src/main.cpp
    src/CertificateHandler.cpp)


find_package(OpenSSL REQUIRED)

# FIXME: Remove this by properly integrating openSSL into cmake    
#target_include_directories(FWMManager PRIVATE 
#    ${CMAKE_SOURCE_DIR}/../../openssl-local/include)

#target_link_directories(FWMManager PRIVATE ${CMAKE_SOURCE_DIR}/../../openssl-local/lib64)


# Link with libraries
target_link_libraries(FWMManager PRIVATE 
    DemoLib
#    crypto
#    ssl
    OpenSSL::SSL
    OpenSSL::Crypto
)

#
# Tests
#
add_executable(FWMManagerTest
    src/CertificateHandler.cpp
    test/CertificateHandlerTest.cpp
    )

add_compile_definitions(TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/data")


# Link with libraries
target_link_libraries(FWMManagerTest PRIVATE Catch2::Catch2WithMain OpenSSL::Crypto OpenSSL::SSL)

# for coverage: ensure tests are executed in debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(FWMManagerTest PRIVATE --coverage)
    target_link_options(FWMManagerTest PRIVATE --coverage)
endif()

target_include_directories(FWMManagerTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

add_test(NAME FWMManagerTest COMMAND FWMManagerTest)

# Custom target to run tests and generate coverage report 
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_target(coverageFWManager 
    COMMAND ${CMAKE_CTEST_COMMAND} -T test --output-on-failure 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --xml -o coverage.xml 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --html --html-details -o coverage.html
    DEPENDS FWMManagerTest )
endif()
