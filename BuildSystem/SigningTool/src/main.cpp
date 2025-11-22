#include <iostream>
#include "P11Wrapper.h"

char const *MY_MSG = "This is my message to sign.";
char const *MY_MSG2 = "This is my counterfeit message.";

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <PIN>\n";
        return 1;
    }

    char const *pin=argv[1];

    try
    {
        p11::Wrapper w(pin);
        unsigned char signature[256];
        unsigned int signatureSize = sizeof(signature);
        if (w.signBuffer((const unsigned char *)MY_MSG, strlen(MY_MSG), signature, &signatureSize) != 0)
        {
            std::cerr << "Failed to sign input\n";
        }
        else
        {
            if (w.verifySignature((const unsigned char *)MY_MSG, strlen(MY_MSG), signature, signatureSize) != 0)
            {
                std::cerr << "Failed to verify signature\n";
            }

            if (w.verifySignature((const unsigned char *)MY_MSG2, strlen(MY_MSG2), signature, signatureSize) == 0)
            {
                std::cerr << "Failed to detect invalid signature\n";
            }
        }

    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
