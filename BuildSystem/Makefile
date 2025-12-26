
#
# User PIN, object ID for key and cert on Crypto Token
#
TOKEN_USER_PIN 			:= "123456"
TOKEN_OBJECT_ID_SSH		:= "01"
SSH_LABEL				:= "SSH"
TOKEN_OBJECT_ID 		:= "02"
TOKEN_SIGNING_LABEL 	:= "Build_Signing"

#
# Where was libp11 installed? see README.md
#
LIB_P11_FOLDER			:= $(shell echo ~)/libp11-local

#
# Tools, shared objects
#
OPENSSL 				:= $(shell which openssl)
PKCS_TOOL 				:=$(shell which pkcs11-tool)
PKCS_MODULE 			:= $(shell find /usr/lib -name "opensc-pkcs11.so" -type f)
# these params must always be provided eith PKCS_TOOL, use this shorthand
PKCS_PARAMS 			:= --module $(PKCS_MODULE) --login --pin $(TOKEN_USER_PIN)

#
# Options for the compilation of the signing-tool
#
CPP = g++
OPENSSL_INCLUDE 		:= $(shell pkg-config --variable=includedir openssl)
OPENSSL_LIB 			:= $(shell pkg-config --variable=libdir openssl)
HOME_FOLDER 			:= $(shell echo ~)
LIBP11_INCLUDE 			:= $(LIB_P11_FOLDER)/include
LIBP11_LIB	 			:= $(LIB_P11_FOLDER)/lib
CCOPTS 					:= -O0 -ggdb3 -Wall -Wextra -Werror -ISigningTool/src -I$(LIBP11_INCLUDE) -I$(OPENSSL_INCLUDE)
LINKOPTS				:= -L$(LIBP11_LIB) -L$(OPENSSL_LIB) -lp11 -lcrypto

all: CA_Certificate.pem Build_Certificate.pem .token_Cert Mgmnt_Certificate.pem signing-tool

#
# Root Certification Authority Key pair (CA certificate) ------------------------------------------
#

# certificate
CA_Certificate.pem: CA_priv.pem
	$(OPENSSL) req -x509 -new -nodes \
    -key CA_priv.pem \
    -sha256 -days 365 \
    -subj "/CN=MyRootCA/O=MyOrg/C=AT" \
    -out $@

# key pair
CA_priv.pem:
	$(OPENSSL) genpkey -algorithm ed25519 -out $@


#
# Build certificate and key pair ------------------------------------------------------------------
#

# download certificate onto token
.token_Cert: Build_Certificate.der
	$(PKCS_TOOL) $(PKCS_PARAMS) --write-object Build_Certificate.der --type cert --id $(TOKEN_OBJECT_ID) --label $(TOKEN_SIGNING_LABEL)
	touch $@

# build certificate in DER format, for downloading
Build_Certificate.der: Build_Certificate.pem
	$(OPENSSL) x509 -in Build_Certificate.pem -outform DER -out $@

# certificate in PEM format
Build_Certificate.pem: Build_csr.pem CA_Certificate.pem CA_priv.pem BuildSignReq.conf
	$(OPENSSL) x509 -req -in Build_csr.pem -CA CA_Certificate.pem -CAkey CA_priv.pem \
	-CAcreateserial -days 365 \
	-extensions v3_req -extfile BuildSignReq.conf \
	-out $@

# certificate signing request
Build_csr.pem: .token_Key BuildSignReq.conf
	$(OPENSSL) req -engine pkcs11 -keyform engine -key "pkcs11:object=$(TOKEN_SIGNING_LABEL);type=private" -config BuildSignReq.conf -new -out $@

# private/public key pair, created directly on token
.token_Key:
	$(PKCS_TOOL) $(PKCS_PARAMS) --keypairgen --key-type rsa:2048 --id $(TOKEN_OBJECT_ID) --label $(TOKEN_SIGNING_LABEL)
	touch $@

#
# FW Mgmnt certificate and key pair ---------------------------------------------------------------
#

Mgmnt_Certificate.pem: Mgmnt_csr.pem CA_Certificate.pem CA_priv.pem MgmntSignReq.conf
	$(OPENSSL) x509 -req -in Mgmnt_csr.pem -CA CA_Certificate.pem -CAkey CA_priv.pem \
	-CAcreateserial -days 365 \
	-extensions v3_req -extfile MgmntSignReq.conf \
	-out $@

Mgmnt_csr.pem: Mgmnt_priv.pem MgmntSignReq.conf
	$(OPENSSL) req -new -key Mgmnt_priv.pem -config MgmntSignReq.conf -out $@

Mgmnt_priv.pem:
	$(OPENSSL) genpkey -algorithm ed25519 -out $@

#
# signing-tool ------------------------------------------------------------------------------------
#

signing-tool: SigningTool/src/main.cpp
	echo "#pragma once" > SigningTool/src/config.h
	echo "#define PKCS_MODULE \"$(PKCS_MODULE)\"" >> SigningTool/src/config.h
	echo "#define TOKEN_SIGNING_LABEL \"$(TOKEN_SIGNING_LABEL)\"" >> SigningTool/src/config.h
	$(CPP) $(CCOPTS) SigningTool/src/main.cpp -o $@ $(LINKOPTS)

#
# test signing-tool -------------------------------------------------------------------------------
#
test: signing-tool Build_Certificate.pem
	echo "This is a test" > signedFile.txt
	signing-tool $(TOKEN_USER_PIN) signedFile.txt signedFile.txt.sig
	$(OPENSSL) x509 -in Build_Certificate.pem -pubkey -noout > Build_pub.pem
	echo "This one should pass:"
	$(OPENSSL) dgst -verify Build_pub.pem -signature signedFile.txt.sig signedFile.txt
	echo "This is a test (forged)" > signedFile.txt
	echo "This one should fail:"
	$(OPENSSL) dgst -verify Build_pub.pem -signature signedFile.txt.sig signedFile.txt

#
# Cleanup -----------------------------------------------------------------------------------------
#
clean: clean_token_key clean_token_cert
	rm -rf *.pem *.srl *.der signing-tool signedFile.txt signedFile.txt.sig

#
# Remove certs, keys from crypto token, ignore errors if objects were not found
#
.IGNORE: clean_token_key clean_token_cert newSSHKey

clean_token_key:
	rm -f .token_Key && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(TOKEN_SIGNING_LABEL) --type privkey
	rm -f .token_Key && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(TOKEN_SIGNING_LABEL) --type pubkey

clean_token_cert:
	rm -f .token_Cert && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(TOKEN_SIGNING_LABEL) --type cert


#
# Renews SSH key pair on crypto token, or creates a new one. Exports and converts the public key to ssh format
#
sshkey:
	rm -f .token_Key && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(SSH_LABEL) --type privkey
	rm -f .token_Key && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(SSH_LABEL) --type pubkey
	$(PKCS_TOOL) $(PKCS_PARAMS) --keypairgen --key-type rsa:2048 --id $(TOKEN_OBJECT_ID_SSH) --label $(SSH_LABEL)
	$(PKCS_TOOL) $(PKCS_PARAMS) --read-object --type pubkey --id $(TOKEN_OBJECT_ID_SSH) --output ssh_pub.der
	$(OPENSSL) rsa -inform DER -pubin -in ssh_pub.der -outform PEM -out ssh_pub.pem
	ssh-keygen -i -m PKCS8 -f ssh_pub.pem > id_rsa.pub
	rm -f ssh_pub.der ssh_pub.pem
