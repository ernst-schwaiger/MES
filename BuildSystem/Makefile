
#
# User PIN, object ID for key and cert on Crypto Token
#
TOKEN_USER_PIN := "123456"
TOKEN_OBJECT_ID := "02"
TOKEN_SIGNING_LABEL := "MES_Signing"


#
# Tools, shared objects
#
OPENSSL := $(shell which openssl)
PKCS_TOOL :=$(shell which pkcs11-tool)
PKCS_MODULE := $(shell find /usr/lib -name "opensc-pkcs11.so" -type f)
# these params must always be provided, use this shorthand
PKCS_PARAMS := --module $(PKCS_MODULE) --login --pin $(TOKEN_USER_PIN)

all: CA_Cert.pem MES_Cert.pem .token_Key .token_Cert signing-tool

#
# Generate CA Keys and Certificate
#
CA_Cert.pem: CA_priv.pem
	$(OPENSSL) req -x509 -new -nodes \
    -key CA_priv.pem \
    -sha256 -days 365 \
    -subj "/CN=MyRootCA/O=MyOrg/C=AT" \
    -out $@

CA_priv.pem:
	$(OPENSSL) genrsa -out $@

#
# Generate MES Signing Keys and Certificate
#
MES_priv.pem:
	$(OPENSSL) genrsa -out $@

MES_csr.pem: MES_priv.pem MESSignReq.conf
	$(OPENSSL) req -new -key MES_priv.pem -config MESSignReq.conf -out $@

MES_Cert.pem: MES_csr.pem CA_Cert.pem CA_priv.pem MESSignReq.conf
	$(OPENSSL) x509 -req -in MES_csr.pem -CA CA_Cert.pem -CAkey CA_priv.pem \
	-CAcreateserial -days 365 \
	-extensions v3_req -extfile MESSignReq.conf \
	-out $@
#
# Put MES Signing Keys and Certificate onto Crypto Token
#
.token_Key: MES_priv.der
	$(PKCS_TOOL) $(PKCS_PARAMS) --write-object MES_priv.der --type privkey --id $(TOKEN_OBJECT_ID)	--label $(TOKEN_SIGNING_LABEL)
	touch $@

MES_priv.der: MES_priv.pem
	$(OPENSSL) rsa -in MES_priv.pem -outform DER -out $@

.token_Cert: MES_Cert.der
	$(PKCS_TOOL) $(PKCS_PARAMS) --write-object MES_Cert.der --type cert --id $(TOKEN_OBJECT_ID) --label $(TOKEN_SIGNING_LABEL)
	touch $@

MES_Cert.der: MES_Cert.pem
	$(OPENSSL) x509 -in MES_Cert.pem -outform DER -out $@

#
# Build a tool for signing binaries, compile labels and module path into it
#
signing-tool:
	PKCS_MODULE=$(PKCS_MODULE) TOKEN_SIGNING_LABEL=$(TOKEN_SIGNING_LABEL) make -C SigningTool && cp SigningTool/signing-tool .

#
# Cleanup certs and keys, also remove key and cert from crypto token
#
clean: clean_token_key clean_token_cert
	rm -rf *.pem *.srl *.der signing-tool
	make -C SigningTool clean

#
# Remove certs, keys from crypto token, ignore errors if objects were not found
#
.IGNORE: clean_token_key clean_token_cert

clean_token_key:
	rm -f .token_Key && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(TOKEN_SIGNING_LABEL) --type privkey

clean_token_cert:
	rm -f .token_Cert && $(PKCS_TOOL) $(PKCS_PARAMS) --delete-object --label $(TOKEN_SIGNING_LABEL) --type cert
