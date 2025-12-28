#!/bin/bash

# pick the first PKCS provider available on the system
PKCS11_PROVIDER="$(dpkg -L opensc-pkcs11 | grep opensc-pkcs11.so | head -n 1)"
if [ ! -f "${PKCS11_PROVIDER}" ]; then
    echo "PKCS11_PROVIDER is not installed, pls do \"sudo apt install p11-kit libp11-kit-dev\""
    exit 1
fi

# the signing-tool is supposed to be in the same folder as this script
SIGNING_TOOL="$(dirname "$0")/signing-tool"
if [ ! -f "${SIGNING_TOOL}" ]; then
    echo "${SIGNING_TOOL} not found, has it been built already?"
    exit 1
fi

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <package-path> <user@fw-management-host>"
    exit 1
fi

PACKAGE=$1
FIRMWARE_USER_AT_SERVER=$2

if [ ! -f "${PACKAGE}" ]; then
    echo "Could not find package ${PACKAGE}"
    exit 1
fi 

# PIN for signing update package
read -r -s -p "Enter PIN: " PIN
echo

# Sign the passed package with private key in the crypto dongle
if ! "./${SIGNING_TOOL}" "${PIN}" "${PACKAGE}" "${PACKAGE}.sig" || [ ! -f "${PACKAGE}.sig" ]; then
    echo "Signing package ${PACKAGE} failed."
    exit 1
fi

# copy package and signature to firmware server
if ! scp -o "PKCS11Provider=${PKCS11_PROVIDER}" "${PACKAGE}" "${PACKAGE}.sig" "${FIRMWARE_USER_AT_SERVER}:~/FWManager/FirmwareUpdateIn" ; then
    echo "Failed to copy package and signature to ${FIRMWARE_USER_AT_SERVER}:~/FWManager/FirmwareUpdateIn"
    exit 1
fi

echo "Successfully copied ${PACKAGE} and ${PACKAGE}.sig to ${FIRMWARE_USER_AT_SERVER}:~/FWManager/FirmwareUpdateIn"
exit 0
