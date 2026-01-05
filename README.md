# MES
Mobile and Embedded Security

## Build

Prerequisites:

```bash
sudo apt update
sudo apt install -y g++ gcc cmake make gcovr pkg-config
```

For building the design document:
```bash
sudo apt install -y texlive latexmk texlive-latex-extra
```

Editing LaTeX in VS code: Recommend installation of LaTeX Workshop extension from James Yu.

## Building Software on BuildSystem

See [Readme.md](BuildSystem/README.md) in BuildSystem folder.

## Building/Flashing/Running Client
Initialize the RIOT repository
```bash
git submodule init
git submodule update
```

Build the application:
```bash
make all
```

Flash the application:
```bash
make flash
```

Enter the client's terminal:
```bash
make term
```

## Building/Running FWManager

To build the `FWManager` binary:

```bash
pushd FWManager && \
make && \
popd
```

Run the `FWManager` server with the path to the configuration folder and a path to a python script, e.g.:
```bash
./FWManager/FWManager ~/FWManagerConfigFolder ~/Python/processSignedFirmwarePackage.py
```

The `./FWManager/FWManager` service expects at least the following files/subfolders in the passed configuration folder:

| File | Comment |
| - | - |
|certificates/CA_Certificate.pem|Root/CA Certificate to verify Build_Certificate.pem, Mgmnt_Certificate.pem|
|certificates/Build_Certificate.pem|Certificate for verifying signatures of incoming firmware update packages|
|certificates/Mgmnt_Certificate.pem|Certificate for signing outgoing SUIT packages|
|certificates/Mgmnt_priv.pem|Private key for signing outgoing SUIT packages|
|FirmwareUpdateIn|Folder which contains incoming firmware update packages and their signature files|

The configuration folder is created by running `make` in the `BuildSystem` folder, see also the section 
"Generate keys and certificates, signing-tool, Install on Firmware Management Server" in the Build System [Readme.md](BuildSystem/README.md).

The server logs messages to `/var/log/syslog`, which can be retrived via `sudo tail -f /var/log/syslog`, or via
`sudo journalctl -t FWManager` (which filters out all syslog messages not related to the server).

## ToDos

## Agenda 25-12-28

- Generate private/public key pair for two client nodes in Build System?
-> generate private/public key pairs for Client1 and Client2 (as .pem or .der file) (Stefan)

- Document build of FWManager binary on buster properly, e.g. ~/.bashrc config != Build System (Ernst)
- Which Raspbian Buster version to use for a working IEEE 802.15.4 and 6LoWPAN stack? I used this one:
https://downloads.raspberrypi.com/raspios_arm64/images/raspios_arm64-2021-05-28/2021-05-07-raspios-buster-arm64.zip
(For this one, no updates are available, OpenSSL 3.x must be checked out and built)
- Clarify hand-over FWManager invokes python script: which parameters?
- Remove CMake from FWManager, not necessary
- Manifest file structure?
- Raspbian Buster - 
- Discuss common Folder for FW Management, e.g.
    - FWManager
        - certificates
            - Build_Certificate.der  
            - CA_Certificate.pem  
            - CA_priv.pem
            - Mgmnt_priv.pem
            - Build_Certificate.pem
            - CA_Certificate.srl
            - Mgmnt_Certificate.pem
        - FirmwareUpdateIn
            - packageName.tgz
            - packageName.tgz.sig
        - FirmwareUpdateProcessing(?)
            - ...
        - Clients
            - Client1
                - PubKey.pem
                - ...
AI Ernst: Remove Clent folder from repo.
DONE
AI Ernst: Generate sequence number in manifest file
DONE

# Agenda 26-01-04

- MES/BuildSystem/Makefile: Add target to create ECC public/private key pairs in MES folder
- AI Ernst: Try to get 6LoWPAN to run on Raspberry device.


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