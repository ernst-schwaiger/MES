#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "P11Wrapper.h"

using namespace std;

constexpr size_t SIGNATURE_LEN_BYTES = 256U;

typedef array<uint8_t, SIGNATURE_LEN_BYTES> SignatureType;


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

int signFile(string const &path, p11::Wrapper const &w)
{
    size_t fileLength;
    int ret = getFileLength(path, fileLength);
    if (ret != 0)
    {
        return ret;
    }

    ifstream file(path, ios::binary);
    vector<uint8_t> buf(fileLength);

    if (!file.read(reinterpret_cast<char *>(buf.data()), fileLength)) 
    {
        return 1;
    }

    SignatureType outSignature;
    unsigned int outSigLen = outSignature.size();
    if (w.signBuffer(buf.data(), buf.size(), outSignature.data(), &outSigLen))
    {
        return 1;
    }

    // write signature
    string outpath = path + ".sig";
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

    if (argc != 3) 
    {
        cerr << "usage: " << argv[0] << "<PIN> <file> \n";
        return 1;
    }

    string pin=argv[1];
    string inFile=argv[2];

    try
    {
        p11::Wrapper wrapper(pin.c_str());
        ret = signFile(inFile, wrapper);
        if (ret != 0)
        {
            cerr << "Failed to sign file " << inFile << ".\n";
        }
    }
    catch(const runtime_error& e)
    {
        cerr << e.what() << '\n';
    }
    
    return ret;
}
