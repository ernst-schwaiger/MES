#!/bin/bash

SIGNING_TOOL="signing-tool"
FIRMWARE_USER_AT_SERVER="ernst@10.147.78.1"

PKCS11_PROVIDER="$(find /usr/lib/ -name 'opensc-pkcs11.so' | head -n 1)"


#FIXME: Derive path of shell script, use it to invoke SIGNING-TOOL
if [ ! -f "${PKCS11_PROVIDER}" ]; then
    echo "PKCS11_PROVIDER is not installed, pls do \"sudo apt install p11-kit libp11-kit-dev\""
    exit 1
fi

if [ ! -f "./${SIGNING_TOOL}" ]; then
    echo "${SIGNING_TOOL} not found, please build it"
    exit 1
fi

if [ "$#" -ne 1 ]; then
    echo "$#"
    echo "Usage: $0 <package-path>"
    exit 1
fi

PACKAGE=$1

if [ ! -f ${PACKAGE} ]; then
    echo "Could not find package ${PACKAGE}"
    exit 1
fi 

# start the ssh agent unless already started, or use systemd
# to start the agent
if ! ssh-add -l >/dev/null 2>&1; then
    echo "Starting new ssh-agent"
    eval "$(ssh-agent -s)"
fi

# add PKCS provider, will prompt for PIN
ssh-add -s "${PKCS11_PROVIDER}"


echo "PIN:"
read PIN
echo "Entered PIN is : $PIN"

./${SIGNING_TOOL} "${PIN}" "${PACKAGE}" "${PACKAGE}.sig"

if [ $? -ne 0 ] || [ ! -f "${PACKAGE}.sig" ]; then
    echo "Signing package ${PACKAGE} failed."
    exit 1
fi

# copy package and signature to firmware server
scp -o "PKCS11Provider=${PKCS11_PROVIDER}" "${PACKAGE}" "${FIRMWARE_USER_AT_SERVER}":~
scp -o "PKCS11Provider=${PKCS11_PROVIDER}" "${PACKAGE}.sig" "${FIRMWARE_USER_AT_SERVER}":~
#scp "${PACKAGE}" "${FIRMWARE_USER_AT_SERVER}":~
#scp "${PACKAGE}.sig" "${FIRMWARE_USER_AT_SERVER}":~
