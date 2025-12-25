#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

#include <cstdlib>

#include <unistd.h> // sleep()

#include "CertificateHandler.h"

using namespace std;
using namespace fwm;

static void usage(char *argv0)
{
    cerr << "Usage: " << argv0 << " <config folder>\n";
    exit(1);
}

vector<filesystem::path> find_sig_files(const filesystem::path& dir) 
{
    vector<filesystem::path> result;

    for (const auto& entry : filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sig") {
            result.push_back(entry.path());
        }
    }

    return result;
}

static void pollForSignatureFiles(filesystem::path const &firmwareUpdateInFolder, CertificateHandler const &certHandler)
{
    string pythonScript = firmwareUpdateInFolder.parent_path() / "python" / "myScript.py";

    vector<filesystem::path> sigFiles;
    do
    {
        sigFiles = find_sig_files(firmwareUpdateInFolder);
        sleep(5);
        cout << "No signature files found.\n";
    } while (sigFiles.empty());

    for (auto const &sigFile : sigFiles)
    {
        string sigFileName = string(sigFile.filename().c_str());
        size_t sigFileNameLen = sigFileName.length();

        if (sigFileNameLen > 4)
        {
            string checkFileName = sigFileName.substr(0, sigFileNameLen - 4);
            filesystem::path checkFile = sigFile.parent_path() / checkFileName;

            if (certHandler.checkSignatureWithLeafCert(checkFile, sigFile))
            {
                cout << "File " << checkFile.c_str() << " was successfully verified against signature " << sigFile.c_str() << "\n";
                string pythonCall = "python3 " + pythonScript + " " + string(checkFile.c_str()); 
                system(pythonCall.c_str());
            }
            else
            {
                cout << "Failed to check" << checkFile.c_str() << " against signature " << sigFile.c_str() << "\n";
            }

            filesystem::remove(checkFile);
            filesystem::remove(sigFile);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
    }

    filesystem::path serverFolder = argv[1];

    filesystem::path certFolder = serverFolder / "Certs";

    if (!filesystem::is_directory(certFolder)) 
    {
        cerr << "Path " << certFolder.c_str() << " is not a folder\n";
        usage(argv[0]);
    }

    filesystem::path caCert = certFolder / "CA_Certificate.pem";
    filesystem::path buildCert = certFolder / "Build_Signing.pem";
    filesystem::path mgmntCert = certFolder / "Mgmnt_Signing.pem";

    filesystem::path firmwareUpdateInFolder = serverFolder / "FirmwareUpdateIn";

    try
    {
        CertificateHandler certHandler(caCert, buildCert);

        while(true)
        {
            pollForSignatureFiles(firmwareUpdateInFolder, certHandler);
        }
    }
    catch(const runtime_error& e)
    {
        cerr << e.what() << '\n';
        return 1;
    }
    catch(...)
    {
        cerr << "Unknown exception occurred.\n";
        return 1;
    }

    return 0;
}