# MES
Mobile and Embedded Security: Secure Firmware Update

Students:
- Miguel Arcilla
- Chiara Fedrizzi
- Kilian Hnidek
- Viktor Nawrata
- Stefan Ohnewith
- Ernst Schwaiger

Repository Links:
- MES implementations: https://github.com/ernst-schwaiger/MES.git
- RIOT-OS changes: https://github.com/Cthuuko/RIOT/tree/be2c56fcc14f5245d1e8482a66e522ed368f1c18

## Project Challenges

### Broken Feitian Crypto Tokens

In the project, we decided to use the Feitian ePass2003 Security Token for signing the firmware update package, and for SSH/SCP authentication with the Firmware Update Manager. During the project integration phase it turned out that not all of the tokens were actually working, which delayed the project progress. We recommend to test the crypto tokens upfront before passing them on to the next MES project teams.

### Entering the Feitian PIN

The original plan in the project was to create a script `signAndDeliver.sh`, which will be passed the PIN once, then the script will use that PIN for signing the firmware update package and provide it to the SSH client for transferring files to the Firmware Management Server.

It turned out that - for security reasons - it is not possible to pass that PIN directly to the SSH client via command line, which forces the user in our workflow to enter the PIN twice. As a mitigation, we tried to use the SSH agent to cache the PIN value upfront, but the agent then was interfering with the signing app we built, i.e. only every other PIN authentication attempt of our signing app was successful with the SSH agent running (we still don't know the reason for that). Hence, we decided not to use the SSH agent.

### Radio Module not working with Raspberry Pi 4B
Since the Raspberry Pi 4B has more resources than the Raspberry Pi 3, it was attempted to use the 4B together with the radio module, unfortunately the communication between the client and the Pi 4B was not possible, as described in the Elvis Wiki [6LoWPAN Setup](https://wiki.elvis.science/index.php?title=Raspberry_Pi_3B%2B_802.15.4/6LoWPAN_Setup)

### Speed of the Firmware Download
During the update process, the download speed of the SAMR21-xpro was heavily reliant on the distance between the client and the radio module. A distance further than around 20 cm may have caused a possible update duration up to 15-20 minutes. A closer distance such as 3 cm may have caused a duration of 2-3 minutes.

## Building the Project

### Build System

The software components of the Build System can be installed on any Host running a recent Linux distribution, e.g. Ubuntu 24.04.

#### Build local version of openssl for PKCS support

Install dependencies

```bash
sudo apt install p11-kit libp11-kit-dev pkg-config ninja-build gcc make meson opensc gcc-arm-none-eabi
```

Get the version of the used openssl installation via `openssl version` then download the sources for exactly this version, e.g. 3.0.14 for Ubuntu 22-04 or 3.4.0 for Kali 2025-08-12, configure installation into user home folder with debugging info, compile, install

```
export OPENSSL_VERSION=...
```

```bash
wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz && \
tar -xzf openssl-${OPENSSL_VERSION}.tar.gz && \
pushd openssl-${OPENSSL_VERSION} && \
./Configure --prefix=$HOME/openssl-local --openssldir=$HOME/openssl-local/ssl debug-linux-x86_64 && \
make -sj && make install -sj && \
popd
```

Append the following entries to `$HOME/.bashrc` or `$HOME/.zshrc` for Kali. `openssl` will then run the compiled openssl instead of the installed one. Remove these lines to run the installed openssl version again.

```bash
export LD_LIBRARY_PATH="$HOME/openssl-local/lib64:$LD_LIBRARY_PATH"
export PATH="$HOME/openssl-local/bin:$PATH"
export PKG_CONFIG_PATH="$HOME/openssl-local/lib64/pkgconfig:$PKG_CONFIG_PATH"
```

Open a new terminal so that the settings are taken over, test with `which openssl`. It should display the local path openssl was installed into.

#### Build PKCS provider for openssl

Checkout pkcs11-provider.git provider, build, install it to local folder. A few warnings regarding runtime dependencies appear, they can be ignored.
```bash
git clone https://github.com/latchset/pkcs11-provider.git && \
pushd pkcs11-provider && \
meson setup build --prefix=$HOME/pkcs11-local && \
ninja -C build && \
ninja -C build install && \
popd
```

`$HOME/openssl-local/lib64/ossl-modules` should now contain the shared object of the PKCS provider, `pkcs11.so`.

Find the opensc-pkcs11 driver module that was installed with the dependencies in the first step:
```bash
find /usr/lib -name "opensc-pkcs11.so" -type f
/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so
```
If the driver is installed somewhere else, use that path subsequently

Add the `pkcs11`, `pkcs11_sect` entries to `$HOME/openssl-local/ssl/openssl.cnf`. Replace `/home/ernst/openssl-local` with the path to your local openssl installation, also remove the comment from the `activate = 1` entry in the `[default_sect]`:

```
# List of providers to load
[provider_sect]
default = default_sect
pkcs11 = pkcs11_sect
...
[pkcs11_sect]
module = /home/ernst/openssl-local/lib64/ossl-modules/pkcs11.so
pkcs11-module-path = /usr/lib/x86_64-linux-gnu/pkcs11/opensc-pkcs11.so
activate = 1
...
[default_sect]
activate = 1
```

`openssl list -providers -provider pkcs11` should now list the provider for pkcs11. Ensure the crypto token is plugged in, and that in the VM the "Devices/USB", an "FS USB Token" shows up and is checked.
`pkcs11-tool --module /usr/lib/x86_64-linux-gnu/opensc-pkcs11.so --list-objects` should list the objects (keys, certs, files) on the token.

#### Build libp11

Download released tarball from github, e.g. [libp11-0.4.16.tar.gz](https://github.com/OpenSC/libp11/releases/download/libp11-0.4.16/libp11-0.4.16.tar.gz), extract it, cd into the folder and configure it with the locally compiled openssl version with debugging info, install it below the $HOME folder again.

```bash
wget https://github.com/OpenSC/libp11/releases/download/libp11-0.4.16/libp11-0.4.16.tar.gz && \
tar -xf libp11-0.4.16.tar.gz && \
pushd libp11-0.4.16 && \
LDFLAGS=-L$HOME/openssl-local/lib64 CFLAGS=-I$HOME/openssl-local/include ./configure --prefix=$HOME/libp11-local --enable-debug CFLAGS="-g -O0" && \
make -sj && make install && \
popd
```

In `$HOME/.bashrc` or `$HOME/.zshrc` adapt `LD_LIBRARY_PATH` and `PKG_CONFIG_PATH` so `libp11.so.3` can be found, and `pkg-confg` can find include and library paths

```bash
...
export LD_LIBRARY_PATH="$HOME/openssl-local/lib64:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$HOME/libp11-local/lib:$LD_LIBRARY_PATH"
...
export PKG_CONFIG_PATH="$HOME/openssl-local/lib64/pkgconfig:$PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="$HOME/libp11-local/lib/pkgconfig:$PKG_CONFIG_PATH"
```
#### Setup the Build System

##### Upfront Checks

Before executing any of the steps below, verify in `MES/BuildSystem/Makefile` that 
- `TOKEN_USER_PIN` is correct
- `FIRMWARE_HOST` is the name or IP of the Firmware Management Server host
- `FIRMWARE_USER` is a valid user on the Firmware Management Server host
- the crypto token is plugged in and enabled for the virtual machine

##### Generate SSH Key Pair, Install on Firmware Management Server

- cd into `MES/BuildSystem`
- run `make sshkey`. This will produce a key pair on the token and `id_rsa.pub` in the current folder
- run `make install_sshkey` to install `id_rsa.pub` on the server
- run `make agent_sshkey` to let the SSH agent provide the token PIN to ssh/scp (reduces times to enter PIN on command line)
- to test whether the ssh key works, run `make ssh_with_key`. This should ask for the PIN of the token, then open an ssh on the `FIRMWARE_HOST`.

##### Generate ED25519 private/public key pairs for client nodes
- cd into `MES/BuildSystem`
- run `make client_keypairs`, which will create the following ED25519 key pairs in the `MES` folder 
    - `Client1_priv.pem`, `Client1_pub.pem` and
    - `Client2_priv.pem`, `Client2_pub.pem`

##### Generate keys and certificates, signing-tool, Install on Firmware Management Server

- cd into `MES/BuildSystem`
- run `make` to build
    - the certificate and keys for CA root
    - the Build certificate (signed by CA root) and keys
    - the Mgmnt certificate (signed by CA root) and keys
    - the `signing-tool` for signing the firmware update packages with the private key of the Build certificate
- run `make test` to test the `signing-tool` works properly. The first signing attempt must pass, the second one must fail
- if anything went wrong, do a `make clean && make`, which will recreate all certificates and keys
- run `make install_certs`, which will copy the necessary certificates and keys on the Firmware Management Server in `FIRMWARE_USER@FIRMWARE_HOST:~/FWManager`

##### Deploy Firmware Update to Firmware Management Server

- adapt `MSG` and `VERSION` in `MES/BuildSystem/FWUpdate/Makefile`
- in the `MES/BuildSystem` folder, do the following steps
  - run `make -C FWUpdate` to create a firmware update package in `MES/BuildSystem/FWUpdate/update.tgz`
  - run `make install_firmware` to deliver the firmware to the Firmware Management Server

### Firmware Management Server

The Firmware Management Server is a Raspberry Pi 3B, with a [Raspbian Buster](https://downloads.raspberrypi.com/raspios_arm64/images/raspios_arm64-2021-05-28/2021-05-07-raspios-buster-arm64.zip) OS which had an [OpenLabs 802.15.4 radio](https://wiki.elvis.science/index.php?title=Raspberry_Pi_3B%2B_802.15.4/6LoWPAN_Setup#Setup/Config_802.15.4/6LoWPAN_on_a_Raspberry). Check out the complete repository into the `${HOME}` folder of the user account configured as `FIRMWARE_USER` in `MES/BuildSystem/Makefile`.

Install dependencies
```bash
# RIOT-OS and Build System dependencies
sudo apt install gcc g++ make meson opensc gcc-arm-none-eabi libssl-dev libudev-dev gcc-multilib python3-serial python3-psutil wget unzip git openocd gdb-multiarch esptool podman-docker clangd clang
```
```bash
# SUIT-specific dependencies
pip3 install pycryptodome
pip3 install --user cbor2 cryptography
pip3 install --user aiocoap[linkheader]>=0.4.1
```
```bash
# Fetch the RIOT repository
git submodule init
git submodule update --remote
```

#### Build Firmware Management Server daemon

In the `${HOME}` folder of the `FIRMWARE_USER`, create a folder `FWManager`, and a folder `FWManager/FirmwareUpdateIn`.
The `certificates` sub folder is created and populated with the required certificates by running `make install_certs` on the build system.

| File | Comment |
| - | - |
|certificates/CA_Certificate.pem|Root/CA Certificate to verify Build_Certificate.pem, Mgmnt_Certificate.pem|
|certificates/Build_Certificate.pem|Certificate for verifying signatures of incoming firmware update packages|
|certificates/Mgmnt_Certificate.pem|Certificate for signing outgoing SUIT packages|
|certificates/Mgmnt_priv.pem|Private key for signing outgoing SUIT packages|
|FirmwareUpdateIn|Folder which contains incoming firmware update packages and their signature files|

In the `MES` folder, run `make -C FWManager` to create the daemon binary. When executing the daemon, it must be passed the `FWManager` folder and a specific makefile target, i.e.

```bash
~/MES/FWManager/FWManager ~/FWManager encrypt-samr
```

The `FWManager` will poll the folder `FWManager/FirmwareUpdateIn` cyclically for incoming update packages, which will be two files, `<fwupdatepkg>` and `<fwupdatepkg>.sig`. When it detects these files in the folder, it first verifies the validity of `certificates/Build_Certificate.pem`, then verifies the validity of the signature `<fwupdatepkg>.sig` with respect to the firmware update package `<fwupdatepkg>`. If both checks pass, it will invoke the makefile target which was passed via the command line, which will unzip the firmware update package `<fwupdatepkg>`, create a signed and encrypted SUIT update package, and send it to the SAMR21 xPro nodes.

If either the certificate is not valid, or `<fwupdatepkg>.sig` is not a valid signature of `<fwupdatepkg>`, the daemon will report an error and stop further processing. It logs messages to `/var/log/syslog`, which can be retrieved via `sudo tail -f /var/log/syslog`, or via `sudo journalctl -t FWManager` (which filters out all syslog messages not related to the server).

#### Prepare CoAP File Server for SUIT manifest and Firmware
In order for the Client nodes to fetch the update, we need to provide a CoAP File Server. This is done in the folder `~/MES/FWManager`
```bash
aiocoap-fileserver manifests/
```

### Prepare network
The Firmware Management Server has a persistent wpan interface created through this [guide](https://wiki.elvis.science/index.php?title=Raspberry_Pi_3B%2B_802.15.4/6LoWPAN_Setup#Installation_of_Linux_WPAN_tools)

The IP of the Firmware Management Server is always `fe80::1`.

### SAMR21-xpro Node
The unpatched application can be flashed to the SAMR21-xpro from the RIOT folder inside the project under `~/MES/RIOT/examples/advanced/suit_update`
```bash
APP_VER=1 make clean flash -j4
```

The firmware change happens under `~/MES/RIOT/sys/suit/suit.c`:
```C
static int _mes_echo_command(int argc, char **argv)
{
    (void) argc; (void) argv;
    LOG_INFO("HELLO WORLD!\n"); // <-- This value gets changed
    return 0;
}
```

Get access to the terminal to verify the firmware.
```bash
make term
```

To verify that the "old" firmware has been installed, you can enter the shell command `mes_echo`.
The output should be:
```bash
> mes_echo
mes_echo
HELLO WORLD!
```

### Build new version
To trigger a new version, the Build system must detect a new firmware package that starts the make task `encrypt-samr`.
This task does the following things:
- Unzips the new verified FW package
- Create a new ephemeral key pair
- Encrypt the firmware
- Store ephemeral public key in Fileserver folder
- Copy private key of client to RIOT build
- Generate a new manifest in CBOR and JSON formats
- Signs the manifest with the public key of `Mgmnt_priv.pem`
- Notifies devices about updates

This make task can also be manually triggered under under `~/MES/FWManager/suit`:
```bash
# ~/FWManager/suit
NETWORK_PREFIX=fe80::1 make encrypt-samr
```

On success, the terminal where you entered the terminal, this should be the output.
The board will download, decrypt and install the new firmware into the inactive slot. In this case, slot 1.
```bash
suit_worker: downloading "coap://[fe80::1]/suit_manifest.signed"
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

## Resources

- Standards
    - [SUIT "Software Updates for Internet of Things"](https://datatracker.ietf.org/group/suit/about/#autoid-2)
    - [SUIT Manifest Structure](https://datatracker.ietf.org/doc/html/draft-ietf-suit-manifest-34#name-metadata-structure-overview)
    - [CoAp "The Constrained Application Protocol"](https://coap.space/)
    - [CoAp tutorial](https://community.arm.com/cfs-file/__key/telligent-evolution-components-attachments/01-1996-00-00-00-00-53-31/ARM-CoAP-Tutorial-April-30-2014.pdf)
    - [CoMI "CoAp Management Interface"](https://datatracker.ietf.org/doc/html/draft-ietf-core-comi-04)
    - [Yang](https://en.wikipedia.org/wiki/YANG)
    - [CBOR](https://en.wikipedia.org/wiki/CBOR)

- CoAp "Constrained Application Protocol" Libraries
    - https://libcoap.net
    - https://github.com/chrysn/aiocoap

- SUIT example
  - [SUIT example general information](https://github.com/RIOT-OS/RIOT/tree/2025.07/examples/advanced/suit_update)
  - [SUIT native example](https://github.com/RIOT-OS/RIOT/blob/2025.07/examples/advanced/suit_update/README.native.md)
  - [SUIT hardware example](https://github.com/RIOT-OS/RIOT/blob/2025.07/examples/advanced/suit_update/README.hardware.md)

- Flashing in WSL
  - [Attach USB Device in WSL](https://guide.riot-os.org/getting-started/install-wsl/#attach-a-usb-device-to-wsl)

- [Feitian ePass2003 Security Token](https://www.ftsafe.com/store/product/epass2003-pki-token/)
