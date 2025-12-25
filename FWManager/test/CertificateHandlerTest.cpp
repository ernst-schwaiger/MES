#include <CertificateHandler.h>
#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>

using namespace std;
using namespace fwm;

// Demo Unit Test Case
TEST_CASE( "ensure getValue() works properly", "CertificateHandler" )
{

    filesystem::path basePath = TEST_DATA_DIR;
    filesystem::path caCert = basePath / "CA_Cert.pem";
    filesystem::path buildCert = basePath / "Build_Cert.pem";

    CertificateHandler certHandler(caCert, buildCert);

    filesystem::path checkFile = basePath / "testfile.txt";
    filesystem::path checkFileForged = basePath / "testfile2.txt";
    filesystem::path checkFileSig = basePath / "testfile.txt.sig";

    REQUIRE(certHandler.checkSignatureWithLeafCert(checkFile, checkFileSig));
    REQUIRE(certHandler.checkSignatureWithLeafCert(checkFileForged, checkFileSig) == false);
}
