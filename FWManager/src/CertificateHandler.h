#pragma once

#include <string>

#include <openssl/x509.h>

namespace fwm
{

class CertificateHandler
{
public:
    CertificateHandler(std::string const &caCert, std::string const &leafCert);
    ~CertificateHandler();

    bool isCertChainValid() const;
    bool checkSignatureWithLeafCert(std::string const &fileToCheck, std::string const &signatureFile) const;
private:
    X509* load_cert(std::string const &path) const;
    bool is_self_signed(X509* cert) const;
    bool is_time_valid(X509* cert) const;
    bool verifyCaCert() const;

    X509* caCertX509;
    X509* leafCertX509;
};

} // namespace fwm