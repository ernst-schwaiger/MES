# cross-compiler, strip machine code from binary
CC			:= arm-none-eabi-gcc
OBJCPY		:= arm-none-eabi-objcopy
CCOPTS		:= \
 -mtune=cortex-m0plus \
 -march=armv6s-m \
 -ffunction-sections \
 -fdata-sections \
 -fshort-enums \
 -Os \
 -fwrapv \
 -fno-common \
 -fdata-sections \
 -fno-delete-null-pointer-checks \
 -fdiagnostics-color 

# Configure version and message of our app
VERSION="1.0.0"
MSG="Hello, World $(VERSION)"
SEQUENCE_NUMBER=1768046998

CERTIFICATES_DIR 		?= /home/kuuko/MES/BuildSystem/FWManager/certificates
FWMANAGER_PUBLIC_KEY	?= $(CERTIFICATES_DIR)/Mgmnt_Certificate.pem

all: update.tgz

# Build the new SAMR21-xpro Firmware:
# Under ~\RIOT\sys\suit\suit.c change the LOG_INFO output
# Then execute APP_VER=1768046996 BOARD=samr21-xpro SUIT_COAP_SERVER=[2001:db8::1] make all -j4
# APP_VER is the SEQUENCE_NUMBER needed for manifest
update.tgz:
	MSG=$(MSG) VERSION=$(VERSION) SEQUENCE_NUMBER=$(SEQUENCE_NUMBER) ./generator.sh update.c manifest.mf
	# For ethos:
	# SUIT_COAP_SERVER=[2001:db8::1] APP_VER=$(SEQUENCE_NUMBER) make -C ../../RIOT/examples/advanced/suit_update all -j4
	# For 6LoWPAN:
	SUIT_COAP_SERVER=[fe80::1] APP_VER=$(SEQUENCE_NUMBER) make -C ../../RIOT/examples/advanced/suit_update riotboot -j4
	mv ../../RIOT/examples/advanced/suit_update/bin/samr21-xpro/riotboot_files/slot1.$(SEQUENCE_NUMBER).bin update.bin
	tar -czf update.tgz update.bin manifest.mf

clean:
	rm -f update.* manifest.mf
