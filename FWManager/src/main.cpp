#include <iostream>
#include <filesystem>

#include "CertificateHandler.h"

using namespace std;
using namespace fwm;

char const *ROOT_CERT = "CA_Cert.pem";

// FIXME: Change the name of the certs after adaption of cert makefile
//char const *BUILD_CERT = "BuildCert.pem";
char const *BUILD_CERT = "MES_Cert.pem";
char const *MGMNT_CERT = "FWMgmntCert.pem";

static void usage(char *argv0)
{
    cerr << "Usage: " << argv0 << " <config folder>\n";
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
    }

    filesystem::path caCert = argv[1];

    if (!filesystem::is_directory(caCert)) 
    {
        cerr << "Path " << argv[1] << " is not a folder\n";
        usage(argv[0]);
    }
    
    caCert.append(ROOT_CERT);
    if (!filesystem::is_regular_file(caCert)) 
    {
        cerr << "Path " << caCert << " is not a file\n";
        usage(argv[0]);
    }

    filesystem::path buildCert = argv[1];
    buildCert.append(BUILD_CERT);
    if (!filesystem::is_regular_file(buildCert)) 
    {
        cerr << "Path " << buildCert << " is not a file\n";
        usage(argv[0]);
    }

    try
    {
        CertificateHandler certHandler(caCert, buildCert);

        filesystem::path checkfile("/home/ernst/projects/MES/BuildSystem/testfile.txt");
        filesystem::path checkfile2("/home/ernst/projects/MES/BuildSystem/testfile2.txt");
        filesystem::path checkfilesig("/home/ernst/projects/MES/BuildSystem/testfile.txt.sig");

        if (certHandler.checkSignatureWithLeafCert(checkfile, checkfilesig))
        {
            cout << "Signature is valid!\n";
        }
        else
        {
            cout << "Signature is not valid!\n";
        }

        if (certHandler.checkSignatureWithLeafCert(checkfile2, checkfilesig))
        {
            cout << "Signature of counterfeit file is valid!\n";
        }
        else
        {
            cout << "Signature of counterfeit file is not valid!\n";
        }
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown exception occurred.\n";
        return 1;
    }

    return 0;
}