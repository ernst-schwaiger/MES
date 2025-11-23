#pragma once

#include <string.h>
#include <stdexcept>
#include <array>
#include <algorithm>

#include <libp11.h>
#include <openssl/sha.h>

#include "config.h" // generated in Makefile

namespace p11
{

constexpr size_t SIGNATURE_LEN_BYTES = 256U; // RSA-SHA-256 signatures

typedef std::array<uint8_t, SIGNATURE_LEN_BYTES> SignatureType;

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

        int rc = PKCS11_CTX_load(m_ctx, PKCS_MODULE);
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
        // Picks the certificate with the compiled-in signing label
        PKCS11_CERT *cert = getCertificate();
        if (cert == nullptr)
        {
            return -1;
        }

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
    
private:

    PKCS11_CERT *getCertificate() const
    {
        PKCS11_CERT *certs = nullptr;
        PKCS11_CERT *ret = nullptr;
        unsigned int ncerts = 0;

        PKCS11_enumerate_certs(m_tokenSlot->token, &certs, &ncerts);

        for (unsigned int certIdx = 0; certIdx < ncerts; certIdx++)
        {
            if (!strncmp(TOKEN_SIGNING_LABEL, certs[certIdx].label, strlen(TOKEN_SIGNING_LABEL)))
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
    unsigned int m_nslots;    
};

} // namespace p11
