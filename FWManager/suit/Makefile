APPLICATION = suit_gen_manifest

.PHONY: network encrypt-firmware all json create-manifest-basis generate-manifest generate-manifest-json clean

SUIT_TOOL_PATH ?= python3 $(CURDIR)/suit-manifest-generator/bin/suit-tool
MANIFEST_DIR ?= $(CURDIR)/manifests
FIRMWARE_UPDATE_DIR ?= $(HOME)/FWManager/FWUpdateIn

# The Build System Makefile copies its artefacts into the FWManager subfolder in the FIRMWARE_USER home folder,
# the certs and keys are in the certificates subfolder
# The certs folder can also be derived from the path to the firmware update passed to the python script
# folder(<fw_update_file_package>)/../certificates
CERTIFICATES_DIR 		:= $(HOME)/FWManager/certificates
FWMANAGER_PRIVATE_KEY	:= $(CERTIFICATES_DIR)/Mgmnt_priv.pem

NETWORK_PREFIX ?= 2001:db8::1

COAP_URL ?= coap://[$(NETWORK_PREFIX)]/
SEQ_NR ?= 1768046996

# Paths for the files
MANIFEST_TEMP_PATH ?= $(MANIFEST_DIR)/suit.tmp
MANIFEST_UNSIGNED_PATH ?= $(MANIFEST_DIR)/manifest
MANIFEST_SIGNED_PATH ?= $(MANIFEST_DIR)/suit_manifest.signed

# Paths for firmware
FIRMWARE_PATH = $(MANIFEST_DIR)/update.bin
FIRMWARE_ENCRYPTED_PATH = $(MANIFEST_DIR)/update-enc.bin

EPHEMERAL_PUBLIC_KEY_PATH = cert/ephemeral-public.pem
EPHEMERAL_PUBLIC_KEY_PATH_BSTR = cert/eph_key_bstr

CERT_PATH = $(MANIFEST_DIR)/cert

network:
	sudo ../../RIOT/dist/tools/tapsetup/tapsetup -c
	sudo ip address add $(NETWORK_PREFIX)/64 dev tapbr0

all: create-manifest-basis-native generate-manifest sign-manifest notify-devices clean
json: create-manifest-basis-native generate-manifest-json clean
encrypt-json: unzip-fw encrypt-firmware create-manifest-basis-samr generate-manifest-json clean
encrypt: unzip-fw encrypt-firmware prepare-ephemeral-key move-keys all
encrypt-samr: unzip-fw encrypt-firmware prepare-ephemeral-key move-keys create-manifest-basis-samr generate-manifest sign-manifest notify-devices clean


prepare-ephemeral-key:
	@cd $(CERT_PATH) && openssl ec -in private-self.pem -outform DER -out priv_key.der
	@cd $(CERT_PATH) && openssl ec -pubin -in ephemeral-public.pem -outform DER -out pub_key.der -pubout

move-keys:
	@cd $(CERT_PATH) && (openssl ec -inform DER -in pub_key.der -pubin -outform DER | xxd -p | tr -d '\n' | cut -c 55-182) > eph_key_bstr
	@cd $(CERT_PATH) && python3 generate_key_file.py -o priv.h -vn priv_key_der "$(shell cd $(MANIFEST_DIR)/cert && openssl ec -inform DER -in priv_key.der -outform DER | xxd -p | tr -d '\n' | cut -c 15-78)"
	@cd $(CERT_PATH) && mv priv.h ../../../../RIOT/examples/advanced/suit_update/priv.h

unzip-fw:
	@tar -xzf $(FIRMWARE_UPDATE_DIR)/update.tgz -C $(MANIFEST_DIR)

encrypt-firmware:
	@python3 $(CURDIR)/encrypt_firmware.py -c $(MANIFEST_DIR)/cert/public-self.pem -eo $(MANIFEST_DIR)/$(EPHEMERAL_PUBLIC_KEY_PATH) -eoc $(EPHEMERAL_PUBLIC_KEY_PATH_BSTR) -fn $(FIRMWARE_ENCRYPTED_PATH) $(FIRMWARE_PATH)

create-manifest-basis-native:
	@python3 $(CURDIR)/gen_manifest.py --urlroot $(COAP_URL) -mf $(MANIFEST_DIR)/manifest.mf -o $(MANIFEST_TEMP_PATH) $(FIRMWARE_ENCRYPTED_PATH):0:ram:0 --ecies ecies.json
create-manifest-basis-samr:
	@python3 $(CURDIR)/gen_manifest.py $(FIRMWARE_ENCRYPTED_PATH):0x1000 $(FIRMWARE_ENCRYPTED_PATH):0x20800 --urlroot $(COAP_URL) -mf $(MANIFEST_DIR)/manifest.mf -o $(MANIFEST_TEMP_PATH) -ogfn $(FIRMWARE_PATH) --ecies ecies.json -C samr21-xpro
generate-manifest:
	@$(SUIT_TOOL_PATH) create -f suit -i $(MANIFEST_DIR)/suit.tmp -o $(MANIFEST_UNSIGNED_PATH)
	@$(SUIT_TOOL_PATH) create -f json -i $(MANIFEST_DIR)/suit.tmp -o $(MANIFEST_UNSIGNED_PATH).json
sign-manifest:
	@$(SUIT_TOOL_PATH) sign -k $(FWMANAGER_PRIVATE_KEY) -m $(MANIFEST_UNSIGNED_PATH) -o $(MANIFEST_SIGNED_PATH)
notify-devices:
	# Add more devices here as needed
	# TODO adjust hardcoded ipv6
	@aiocoap-client -m POST "coap://[fe80::a40b:97ff:fe20:19f%riot0]/suit/trigger" --payload "coap://[2001:db8::1]/suit_manifest.signed"
clean:
	#@rm $(MANIFEST_TEMP_PATH)
	@rm -rf $(CURDIR)/suit-manifest-generator/suit_tool/__pycache__
	#@rm $(CERT_PATH)/priv_key.der
	#@rm $(CERT_PATH)/pub_key.der

generate-manifest-json:
	@$(SUIT_TOOL_PATH) create -f json -i $(CURDIR)/manifests/suit.tmp -o $(CURDIR)/manifests/manifest.json