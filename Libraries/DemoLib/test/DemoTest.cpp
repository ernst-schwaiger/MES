#include <Demo.h>
#include <catch2/catch_test_macros.hpp>

using namespace std;

// Demo Unit Test Case
TEST_CASE( "ensure getValue() works properly", "DemoLib" )
{
    REQUIRE(getValue() == 42U);
}
