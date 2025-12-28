#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <unistd.h> // sleep()
#include <dirent.h>

#include "CertificateHandler.h"

using namespace std;
using namespace fwm;

static void usage(char *argv0)
{
    cerr << "Usage: " << argv0 << " <config folder> <python script>\n";
    exit(1);
}

static string parent(string  const &path)
{
    return path.substr(0, path.rfind('/')); 
}

static string filename(string const &path)
{
    std::size_t pos = path.rfind('/');
    return (pos < path.length()) ? path.substr(path.rfind('/') + 1, path.length()) : "<unknown>";
}

static void remove(string const &path)
{
    std::remove(path.c_str());
}

static std::vector<std::string> list_files(const std::string& path) {
    std::vector<std::string> files;

    DIR* dir = opendir(path.c_str());
    if (!dir) return files;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip "." and ".."
        if (std::string(entry->d_name) == "." ||
            std::string(entry->d_name) == "..")
            continue;

        std::string full = path + "/" + entry->d_name;

        // Check if it's a regular file
        struct stat st;
        if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            files.push_back(full);
        }
    }

    closedir(dir);
    return files;
}

static bool is_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        // stat failed (path doesn't exist or no permissions)
        return false;
    }
    return S_ISDIR(info.st_mode);
}

static vector<string> find_sig_files(const string &dir) 
{
    vector<string> result;

    for (const auto& entry : list_files(dir)) {
        if (entry.substr(entry.length() - 4, entry.length()) == ".sig") 
        {
            result.push_back(entry);
        }
    }

    return result;
}

static void pollForSignatureFiles(string const &firmwareUpdateInFolder, CertificateHandler const &certHandler, string const &pythonScript)
{
    vector<string> sigFiles;
    do
    {
        sigFiles = find_sig_files(firmwareUpdateInFolder);
        sleep(5);
        cout << "No signature files found.\n";
    } while (sigFiles.empty());

    for (auto const &sigFile : sigFiles)
    {
        string sigFileName = filename(sigFile);
        size_t sigFileNameLen = sigFileName.length();

        if (sigFileNameLen > 4)
        {
            string checkFileName = sigFileName.substr(0, sigFileNameLen - 4);
            string checkFile = parent(sigFile) + "/" +  checkFileName;

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

            remove(checkFile);
            remove(sigFile);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        usage(argv[0]);
    }

    string serverFolder = argv[1];
    string pythonScript = argv[2];

    string certFolder = serverFolder + "/certificates";

    if (!is_directory(certFolder)) 
    {
        cerr << "Path " << certFolder.c_str() << " is not a folder\n";
        usage(argv[0]);
    }

    string caCert = certFolder + "/CA_Certificate.pem";
    string buildCert = certFolder + "/Build_Certificate.pem";
    string mgmntCert = certFolder + "/Mgmnt_Certificate.pem";

    string firmwareUpdateInFolder = serverFolder + "/FirmwareUpdateIn";

    try
    {
        CertificateHandler certHandler(caCert, buildCert);

        while(true)
        {
            pollForSignatureFiles(firmwareUpdateInFolder, certHandler, pythonScript);
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
