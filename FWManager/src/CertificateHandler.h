#pragma once

#include <filesystem>

#include <openssl/x509.h>

namespace fwm
{

class CertificateHandler
{
public:
    CertificateHandler(std::filesystem::path const &caCert, std::filesystem::path const &leafCert);
    ~CertificateHandler();

    bool isCertChainValid() const;
    bool checkSignatureWithLeafCert(std::filesystem::path const &fileToCheck, std::filesystem::path const &signatureFile) const;
private:
    X509* load_cert(std::filesystem::path const &path) const;
    bool is_self_signed(X509* cert) const;
    bool is_time_valid(X509* cert) const;
    bool verifyCaCert() const;

    X509* caCertX509;
    X509* leafCertX509;
};

} // namespace fwm