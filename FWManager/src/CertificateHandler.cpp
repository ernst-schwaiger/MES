#include <vector>
#include <fstream>

#include <openssl/x509_vfy.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include "CertificateHandler.h"

using namespace std;
using namespace fwm;

CertificateHandler::CertificateHandler(string const &caCert, string const &leafCert) : caCertX509(nullptr), leafCertX509(nullptr)
{
    // load CA and leaf cert, check validity of CA cert (only validity wrt date)
    caCertX509 = load_cert(caCert);
    if (caCertX509 == nullptr)
    {
        throw runtime_error("CA certificate could not be loaded.");
    }

    if (CertificateHandler::verifyCaCert() == false)
    {
        throw runtime_error("CA certificate is not valid.");
    }

    leafCertX509 = load_cert(leafCert);
    if (leafCertX509 == nullptr)
    {
        throw runtime_error("Leaf certificate could not be loaded.");
    }

    // check certificate chain, i.e. is leaf cert signed by CA cert, is it
    // valid wrt date?
    X509_STORE* store = X509_STORE_new();
    if (!X509_STORE_add_cert(store, caCertX509))
    {
        X509_STORE_free(store);
        throw runtime_error("CA certificate is not valid.");
    }

    X509_STORE_CTX* ctx = X509_STORE_CTX_new();
    X509_STORE_CTX_init(ctx, store, leafCertX509, nullptr);
    bool chainValid = (X509_verify_cert(ctx) == 1);
    X509_STORE_CTX_free(ctx);
    X509_STORE_free(store);

    if (!chainValid)
    {
        throw runtime_error("Certificate chain is not valid.");
    }
}

CertificateHandler::~CertificateHandler()
{
    X509_free(caCertX509);
    X509_free(leafCertX509);
}

X509* CertificateHandler::load_cert(string const &path) const
{
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return nullptr;
    X509* cert = PEM_read_X509(f, nullptr, nullptr, nullptr);
    fclose(f);
    return cert;
}

bool CertificateHandler::is_self_signed(X509* cert) const
{
    // Compare subject and issuer
    if (X509_NAME_cmp(X509_get_subject_name(cert),
                      X509_get_issuer_name(cert)) != 0)
        return false;

    // Verify signature with its own public key
    EVP_PKEY* pkey = X509_get_pubkey(cert);
    if (!pkey) return false;

    int ok = X509_verify(cert, pkey);
    EVP_PKEY_free(pkey);

    return ok == 1;
}

bool CertificateHandler::is_time_valid(X509* cert) const
{
    if (X509_cmp_current_time(X509_get_notBefore(cert)) > 0)
        return false;
    if (X509_cmp_current_time(X509_get_notAfter(cert)) < 0)
        return false;
    return true;
}

bool CertificateHandler::verifyCaCert() const
{
    return (caCertX509 != nullptr) && is_self_signed(caCertX509) && is_time_valid(caCertX509);
}

bool CertificateHandler::checkSignatureWithLeafCert(string const &fileToCheck, string const &signatureFile) const
{
    // file to check signature of
    ifstream f(fileToCheck.c_str(), ios::binary);
    vector<unsigned char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    // signature itself
    std::ifstream s(signatureFile.c_str(), std::ios::binary);
    std::vector<unsigned char> sig((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    // resources for signature verification
    EVP_PKEY* pubkey = X509_get_pubkey(leafCertX509);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    bool ret = false;
    if ((pubkey != nullptr) && (ctx != nullptr))
    {
        // used hashing algorithm in signature, must match the one we were using generating the signatures!
        const EVP_MD* md = EVP_sha256();

        ret =
            EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, pubkey) &&
            EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) &&
            EVP_DigestVerifyFinal(ctx, sig.data(), sig.size());
    }

    // free resources
    EVP_PKEY_free(pubkey);
    EVP_MD_CTX_free(ctx);

    return ret;
}