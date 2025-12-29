APPLICATION = suit_gen_manifest

.PHONY: network encrypt-firmware all json create-manifest-basis generate-manifest generate-manifest-json clean

SUIT_TOOL_PATH ?= python3 $(CURDIR)/suit-manifest-generator/bin/suit-tool
MANIFEST_DIR ?= $(CURDIR)/manifests
FIRMWARE_DIR ?= $(MANIFEST_DIR)/firmwares

NETWORK_PREFIX ?= 2001:db8::1

COAP_URL ?= coap://[$(NETWORK_PREFIX)]/
SEQ_NR ?= 1

# Paths for the files
MANIFEST_TEMP_PATH ?= $(MANIFEST_DIR)/suit.tmp
MANIFEST_UNSIGNED_PATH ?= $(MANIFEST_DIR)/manifest
MANIFEST_SIGNED_PATH ?= $(MANIFEST_DIR)/suit_manifest.signed

# Paths for firmware
FIRMWARE_PATH = $(FIRMWARE_DIR)/payload.bin
FIRMWARE_ENCRYPTED_PATH = $(MANIFEST_DIR)/payload-enc.bin

EPHEMERAL_PUBLIC_KEY_PATH = /cert/ephemeral-public.pem
network:
	sudo ../../RIOT/dist/tools/tapsetup/tapsetup -c
	sudo ip address add $(NETWORK_PREFIX)/64 dev tapbr0

all: create-manifest-basis generate-manifest sign-manifest clean
json: create-manifest-basis generate-manifest-json clean
encrypt-json: encrypt-firmware create-manifest-basis generate-manifest-json clean
encrypt: encrypt-firmware all

encrypt-firmware:
	@python3 $(CURDIR)/encrypt_firmware.py -c $(MANIFEST_DIR)/cert/public-self.pem -eo $(MANIFEST_DIR)$(EPHEMERAL_PUBLIC_KEY_PATH) -eoc $(EPHEMERAL_PUBLIC_KEY_PATH) -fn $(FIRMWARE_ENCRYPTED_PATH) $(FIRMWARE_PATH)

create-manifest-basis:
	@python3 $(CURDIR)/gen_manifest.py --urlroot $(COAP_URL) --seqnr $(SEQ_NR) -o $(MANIFEST_TEMP_PATH) $(FIRMWARE_ENCRYPTED_PATH):0:ram:0 --ecies ecies.json
generate-manifest:
	@$(SUIT_TOOL_PATH) create -f suit -i $(MANIFEST_DIR)/suit.tmp -o $(MANIFEST_UNSIGNED_PATH)
sign-manifest:
	@$(SUIT_TOOL_PATH) sign -k ~/.local/share/RIOT/keys/default.pem -m $(MANIFEST_UNSIGNED_PATH) -o $(MANIFEST_SIGNED_PATH)
clean:
	#@rm $(MANIFEST_TEMP_PATH)
	@rm -rf $(CURDIR)/suit-manifest-generator/suit_tool/__pycache__

generate-manifest-json:
	@$(SUIT_TOOL_PATH) create -f json -i $(CURDIR)/manifests/suit.tmp -o $(CURDIR)/manifests/manifest.json