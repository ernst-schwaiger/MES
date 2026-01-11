# SUIT
### Dependencies

The RIOT-OS dependencies:
```bash
sudo apt install make gcc-multilib python3-serial python3-psutil wget unzip git openocd gdb-multiarch esptool podman-docker clangd clang`
```

SUIT-specific dependencies:
```bash
pip3 install --user cbor2 cryptography
pip3 install --user aiocoap[linkheader]>=0.4.1
```

## Native

RIOT-Native is a board emulation that is locally run within the host system using its resources.

### Networking on host

Set up networking:

```bash
sudo dist/tools/tapsetup/tapsetup -c
sudo ip address add 2001:db8::1/64 dev tapbr0
```

### Mock firmware image server preparation

Create a folder acting as a firmware image server endpoint in the RIOT base folder:
```bash
# In RIOT base folder
mkdir coaproot
```

Place a mock image bin:
```bash
echo "AABBCCDD" > coaproot/payload.bin
```

Start the CoAP server in another shell:
```bash
aiocoap-fileserver coaproot
```

### Manifest preparation

The RIOT example provides a script to generate a basic SUIT manifest.

These tools can be found under `dist/tools/suit/suit-manifest-generator`

#### Create a manifest:

```bash
touch suit.tmp # Temporary file to write auto-generated SUIT skeleton
dist/tools/suit/suit-manifest-generator/bin/suit-tool create -f suit -i suit.tmp -o coaproot/suit_manifest
```

The manifest is saved in CBOR format.

#### To create a human-readable manifest in JSON:

```bash
dist/tools/suit/suit-manifest-generator/bin/suit-tool create -f json -i suit.tmp -o coaproot/suit_manifest.json
```

The RIOT example generates keys under `~/.local/share/RIOT/keys`

#### Sign the manifest:

```bash
dist/tools/suit/suit-manifest-generator/bin/suit-tool sign -k ~/.local/share/RIOT/keys/default.pem -m coaproot/suit_manifest -o coaproot/suit_manifest.signed
```

### Building the example
Build the SUIT update application:
```bash
BOARD=native make -C examples/advanced/suit_update all term
```

Configure networking on emulated device terminal:
```bash
ifconfig 5 add 2001:db8::2/64 # Interface may be different than 5. Check with ifconfig!
```

Verify that the storage location is empty:
```bash
storage_content .ram.0 0 64
```

Fetch the manifest to execute a SUIT update:
```bash
suit fetch coap://[2001:db8::1]/suit_manifest.signed
```

Verify that the storage location has been filled by the new mock image:
```bash
storage_content .ram.0 0 64
```

## SAMR21-xpro
```bash
SUIT_COAP_SERVER=[2001:db8::1] SUIT_CLIENT=[fe80::a40b:97ff:fe20:19f%riot0] BOARD=samr21-xpro make suit/notify
APP_VER=1768046993 BOARD=samr21-xpro SUIT_COAP_SERVER=[2001:db8::1] make suit/publish
```

# Analysis

Analysis has been done via tcpdump:
```bash
sudo tcpdump -i tapbr0 -s 65535 -w output-enc
```

# Errors

### If you work in WSL:

Problem: Wrong File Ending during `git pull` or `git submodule update`
```
/bin/sh: 1: /pathToMESProject/RIOT/dist/tools/genconfigheader/genconfigheader.sh: not found /usr/bin/env: ‘python3\r’: No such file or directory...
```

Solution:
```bash
# This sets the file endings to Unix (LF) when you pull the project again
git config --global core.autocrlf input
```

Problem: SUIT example failed
```
2025-11-02 11:30:36,793 # validating class id
2025-11-02 11:30:36,793 # Comparing 9251dc23-fd9d-503f-94c4-bfb95f3167b1 to bcc90984-fe7d-562b-b4c9-a24f26a3a9cd from manifest
2025-11-02 11:30:36,793 # suit_worker: suit_parse() failed. res=-4
2025-11-02 11:30:36,793 # suit_worker: update failed, hdr invalid
```
Solution:
- When creating SUIT skeleton, the class ID or any other ID may be wrong.
```bash
# Creates the SUIT skeletion
dist/tools/suit/suit-manifest-generator/bin/suit-tool create -f suit -i suit.tmp -o coaproot/suit_manifest

nano suit.tmp

# This is how the SUIT skeleton looks like
{
    "manifest-version": 1,
    "manifest-sequence-number": 1,
    "components": [
        {
            "install-id": [
                "ram",
                "0"
            ],
            "vendor-id": "547d0d746d3a5a9296624881afd9407b",
            # In this case since the class id comparison failed.
            # Exchange the class id from bcc90984-fe7d-562b-b4c9-a24f26a3a9cd to 9251dc23-fd9d-503f-94c4-bfb95f3167b1
            "class-id": "<VALUE-TO-BE-EXCHANGED>",
            "file": "coaproot/payload.bin",
            "uri": "coap://[2001:db8::1]/payload.bin",
            "bootable": false
        }
    ]
}
```