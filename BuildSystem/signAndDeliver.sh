#!/bin/bash

FIRMWARE_USER_AT_SERVER="ernst@192.168.0.111"

# pick the first PKCS provider available on the system
PKCS11_PROVIDER="$(find /usr/lib/ -name 'opensc-pkcs11.so' | head -n 1)"
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

if [ "$#" -ne 1 ]; then
    echo "$#"
    echo "Usage: $0 <package-path>"
    exit 1
fi

PACKAGE=$1

if [ ! -f "${PACKAGE}" ]; then
    echo "Could not find package ${PACKAGE}"
    exit 1
fi 

# start the ssh agent unless already started, or use systemd
# to start the agent
# if ! ssh-add -l >/dev/null 2>&1; then
#     echo "Starting new ssh-agent"
#     eval "$(ssh-agent -s)"
# fi

# add PKCS provider, will prompt for PIN
#ssh-add -s "${PKCS11_PROVIDER}"

# PIN for signing update package
read -r -s -p "Enter PIN: " PIN
echo

# Sign the passed package with private key in the crypto dongle
if ! "./${SIGNING_TOOL}" "${PIN}" "${PACKAGE}" "${PACKAGE}.sig" || [ ! -f "${PACKAGE}.sig" ]; then
    echo "Signing package ${PACKAGE} failed."
    exit 1
fi

# copy package and signature to firmware server
if ! scp -o "PKCS11Provider=${PKCS11_PROVIDER}" "${PACKAGE}" "${PACKAGE}.sig" "${FIRMWARE_USER_AT_SERVER}":~ ; then
    echo "Failed to copy package and signature to Firmware Management Server"
    exit 1
fi

echo "Successfully copied ${PACKAGE} and ${PACKAGE}.sig to Firmware Management Server"
exit 0
