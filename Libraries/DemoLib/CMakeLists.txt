project(DemoLib)

# Create library
add_library(DemoLib
    src/Demo.c
)

# Include headers
target_include_directories(DemoLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#
# Tests
#
add_executable(DemoLibTest
    src/Demo.c
    test/DemoTest.cpp
    )

# Link with libraries
target_link_libraries(DemoLibTest PRIVATE Catch2::Catch2WithMain)

# for coverage: ensure tests are executed in debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(DemoLibTest PRIVATE --coverage)
    target_link_options(DemoLibTest PRIVATE --coverage)
endif()

target_include_directories(DemoLibTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

add_test(NAME DemoLibTest COMMAND DemoLibTest)

# Custom target to run tests and generate coverage report 
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_target(coverage 
    COMMAND ${CMAKE_CTEST_COMMAND} -T test --output-on-failure 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --xml -o coverage.xml 
    COMMAND gcovr -r ${CMAKE_CURRENT_SOURCE_DIR} --html --html-details -o coverage.html
    DEPENDS DemoLibTest )
endif()
