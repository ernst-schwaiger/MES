#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "P11Wrapper.h"

using namespace std;
using namespace p11;

static int getFileLength(string const &path, size_t &fileLength)
{
    struct stat sb;
    int ret = stat(path.c_str(), &sb);
    fileLength = (ret == 0) ? sb.st_size : 0;
    {
        fileLength = sb.st_size;
    }

    return ret;
}

static int signFile(string const &inpath, string const &outpath, p11::Wrapper const &w)
{
    size_t fileLength;
    int ret = getFileLength(inpath, fileLength);
    if (ret != 0)
    {
        return ret;
    }

    ifstream file(inpath, ios::binary);
    vector<uint8_t> buf(fileLength);

    if (!file.read(reinterpret_cast<char *>(buf.data()), fileLength)) 
    {
        return 1;
    }

    SignatureType outSignature;
    unsigned int outSigLen = outSignature.size();

    p11::Wrapper::SignStatus status = w.signBuffer(buf.data(), buf.size(), outSignature.data(), &outSigLen);
    if (status != p11::Wrapper::SignStatus::OK)
    {
        switch(status)
        {
            case p11::Wrapper::SignStatus::CERT_LABEL_NOT_FOUND:
                cerr << "Could not find certificate with label " << TOKEN_SIGNING_LABEL << ".\n";
                break;
            case p11::Wrapper::SignStatus::PRIV_KEY_NOT_FOUND:
                cerr << "Could not get private key of certificate with label " << TOKEN_SIGNING_LABEL << ".\n";
                break;
            case p11::Wrapper::SignStatus::SHA256_FAILURE:
                cerr << "Failed to get SHA256 digest of data to sign.\n";
                break;
            case p11::Wrapper::SignStatus::SIGN_FAILURE:
                cerr << "Sign operation on token failed.\n";
                break;
            default:
                assert(0);
        }

        return 1;
    }

    // write signature
    ofstream outFile(outpath, ios::out | ios::binary | ios::trunc);
    if (!outFile.write(reinterpret_cast<char *>(outSignature.data()), outSigLen))
    {
        return 1;
    } 

    return 0;
}


int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc != 4) 
    {
        cerr << "usage: " << argv[0] << " <PIN> <file-to-sign> <signaturefile>\n";
        return 1;
    }

    string pin=argv[1];
    string inFile=argv[2];
    string sigFile=argv[3];

    try
    {
        cout << "Setting up crypto token...\n";
        p11::Wrapper wrapper(pin.c_str());
        cout << "Signing file " << inFile << "...\n";
        ret = signFile(inFile, sigFile, wrapper);
        if (ret != 0)
        {
            cerr << "Failed to sign file " << inFile << ".\n";
        }
        else
        {
            cout << "Successfully written signature of file " << inFile << " to " << sigFile <<".\n";
        }
    }
    catch(const runtime_error& e)
    {
        cerr << "Error: " << e.what() << '\n';
        ret = 1;
    }
    catch(...)
    {
        cerr << "Unknown error.\n";
        ret = 1;
    }

    return ret;
}
