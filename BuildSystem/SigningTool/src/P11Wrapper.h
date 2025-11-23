#pragma once

#include <string>
#include <stdexcept>
#include <libp11.h>
#include <algorithm>

#include <openssl/sha.h>


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


#ifndef TOKEN_SIGNING_LABEL
    #error "Symbol TOKEN_SIGNING_LABEL is missing"
#endif

#ifndef PKCS_MODULE
    #error "Provide PKCS_MODULE is missing"
#endif

//#define MAX_SIGSIZE 256

//constexpr std::string pkcs_module = TOSTRING(PKCS_MODULE);
static const std::string pkcs_module = "/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so";
static const std::string signing_label = "MES_Signing";

// static uint8_t const ASN_1_DIGEST_INFO[] = 
// {
//     0x30, 0x31,                                                                 // Sequence, total length
//           0x30, 0x0d,                                                           // OID Sequence, length
//                 0x06, 0x09,                                                     // Object identifier, 9 bytes long
//                       0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,     // OID for SHA-256
//                 0x05, 0x00,                                                     // NULL
//           0x04, 0x20                                                            // octet string, 32 bytes long
// };

namespace p11
{

class Wrapper
{
public:
    Wrapper(char const *pin) : 
        m_ctx(nullptr), 
        m_slots(nullptr), 
        m_tokenSlot(nullptr),
        m_nslots(0)
    {
        m_ctx = PKCS11_CTX_new();

        int rc = PKCS11_CTX_load(m_ctx, pkcs_module.c_str());
        if (rc) 
        {
            throw std::runtime_error("Failed to load library.");
        }

        rc = PKCS11_enumerate_slots(m_ctx, &m_slots, &m_nslots);
        if (rc < 0) 
        {
            throw std::runtime_error("No slots available.");
        }
        
        m_tokenSlot = PKCS11_find_token(m_ctx, m_slots, m_nslots);
        if (m_tokenSlot == NULL || m_tokenSlot->token == NULL)
        {
            cleanup();
            throw std::runtime_error("No token available.");
        }

        rc = PKCS11_login(m_tokenSlot, 0, pin);
        if (rc)
        {
            cleanup();
            throw std::runtime_error("Login with PIN failed.");
        }

        int logged_in;
        rc = PKCS11_is_logged_in(m_tokenSlot, 0, &logged_in);
        if (rc || (!logged_in))
        {
            cleanup();
            throw std::runtime_error("Login with PIN failed.");
        }
    }

    virtual ~Wrapper()
    {
        cleanup();
    }

    int signBuffer(unsigned char const *buffer, unsigned int bufSize, unsigned char *signature, unsigned int *sigSize) const
    {
        PKCS11_CERT *cert = getCertificate();
        if (cert == nullptr)
        {
            return -1;
        }

        // pick first certificate
        PKCS11_KEY *authkey = PKCS11_find_key(cert);
        if (authkey == nullptr)
        {
            return -1;
        }

        unsigned char digestBuf[ 256 / 8];
        if (SHA256(buffer, bufSize, &digestBuf[0]) == nullptr)
        {
            return -1;
        }

        if (PKCS11_sign(NID_sha256, digestBuf, sizeof(digestBuf), signature, sigSize, authkey) != 1)
        {
            return -1;
        }

        return 0;
    }

    // int verifySignature(unsigned char const *buffer, unsigned int bufSize, unsigned char const *signature, unsigned int sigSize) const
    // {
    //     PKCS11_CERT *cert = getCertificate();
    //     if (cert == nullptr)
    //     {
    //         return -1;
    //     }

    //     EVP_PKEY *pubkey = X509_get_pubkey(cert->x509);
    //     if (pubkey == nullptr)
    //     {
    //         return -1;
    //     }

    //     if (RSA_verify(NID_sha1, buffer, bufSize, signature, sigSize, (RSA *)EVP_PKEY_get0_RSA(pubkey)) != 1)
    //     {
    //         EVP_PKEY_free(pubkey);
    //         return -1;
    //     }
        
    //     EVP_PKEY_free(pubkey);
    //     return 0;
    // }
    
private:

    PKCS11_CERT *getCertificate() const
    {
        PKCS11_CERT *certs = nullptr;
        PKCS11_CERT *ret = nullptr;
        unsigned int ncerts = 0;

        PKCS11_enumerate_certs(m_tokenSlot->token, &certs, &ncerts);

        for (unsigned int certIdx = 0; certIdx < ncerts; certIdx++)
        {
            if (signing_label == certs[certIdx].label)
            {
                ret = &certs[certIdx];
                break;
            }
        }

        return ret;
    }

    void cleanup()
    {
        PKCS11_release_all_slots(m_ctx, m_slots, m_nslots);
    }

    PKCS11_CTX *m_ctx;
    PKCS11_SLOT *m_slots = nullptr;
    PKCS11_SLOT *m_tokenSlot;
    unsigned int m_nslots = 0;    
};

} // namespace p11
