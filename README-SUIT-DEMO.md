# SUIT

## Prerequisites
- [Attach USB Device in WSL](https://guide.riot-os.org/getting-started/install-wsl/#attach-a-usb-device-to-wsl)


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

### Zip firmware with manifest
```bash
tar -czvf update.tgz update.bin manifest.mf
```

## Build

### Prepare network
In one terminal start
```bash
sudo dist/tools/ethos/setup_network.sh riot0 2001:db8::/64
```

In another terminal, execute:
```bash
sudo ip address add 2001:db8::1/128 dev riot0
```

In the same terminal, start:
### CoAP File Server
```bash
# ~/FWManager/suit
aiocoap-fileserver manifests/
```

### Flash new version
The example firmware update has the sequence number `1768046996`, therefore, choose a version lower than that as `APP_VER`
```bash
# ~/RIOT/examples/advanced/suit_update
APP_VER=1768046993 make clean flash -j4
```

### SAMR21-xpro terminal

```bash
# ~/RIOT/examples/advanced/suit_update
make term
```

Now you should see this output:

```bash
----> ethos: sending hello.
----> ethos: activating serial pass through.
----> ethos: hello reply received
----> ethos: hello reply received
```

To verify that the "old" firmware has been installed, you can enter the shell command `mes_echo`.
The output should be:
```bash
> mes_echo
mes_echo
HELLO WORLD!
```

### Build new version
To trigger a new version, execute this command in a new terminal.
This task does the following things:
- Create a new ephemeral key pair
- Store ephemeral public key in Fileserver folder
- Copy private key of client to RIOT build
- Generate a new manifest in CBOR and JSON formats
- Notifies devices about updates
```bash
# ~/FWManager/suit
make encrypt-samr
```

On success, the terminal where you entered the terminal, this should be the output.
The board will download, decrypt and install the new firmware into the inactive slot. In this case, slot 1.
```bash
suit_worker: downloading "coap://[2001:db8::1]/suit_manifest.signed"
suit_worker: got manifest with size 480
suit: verifying manifest signature
...
riotboot_flashwrite: initializing update to target slot 1
Fetching firmware |███████                  |  28%
```

After successful update, the device will restart and start the firmware image with the newer version.
In this case, the firmware that was just decrypted and installed.
```bash
suit_worker: update successful
suit_worker: rebooting...
----> ethos: hello received
Failed to send flush request: Operation not permitted
gnrc_uhcpc: Using 5 as border interface and 0 as wireless interface.
gnrc_uhcpc: only one interface found, skipping setup.
main(): This is RIOT! (Version: f2a59)
RIOT SUIT update example application
Running from slot 1
Image magic_number: 0x544f4952
Image Version: 0x69624194
Image start address: 0x00020900
Header chksum: 0xb1f4519a

Starting the shell
```

To verify that a new firmware has been installed, you can enter the shell command `mes_echo`.
The output should be:
```bash
> mes_echo
mes_echo
I HAVE BEEN UPDATED!
```

### Notify device
This task notifies the devices for updates
```bash
# ~/FWManager/suit
make notify-devices
```

