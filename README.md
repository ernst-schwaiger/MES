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

Release: Optimizations turned on
```bash
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

Debug: Optimizations turned off, debugging symbols are included
```bash
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j
```

Run the application: 
```bash
./release/FWManager/FWManager
./debug/FWManager/FWManager
```

### Run unit tests:
Either `ctest --output-on-failure` or run a test binary, e.g. `./debug/Libraries/DemoLib/DemoLibTest`.

### Generate a Test Coverage Report:
FIXME: Does not work properly yet
Build the debug binary, then run
```bash
cmake --build debug --target coverage --config Debug
```
The coverage report is available in `./debug/DemoLib/coverage.html`

## Develop on remote node using local VS Code

VS Code supports to connect to a remote node, and to execute as if VS code was running on that remote machine, including compilation, debugging, code editing,...

- Preconditions: SSH server executing on the remote host, using ssh keys for authentication
- Install "Remote Development" extension pack from the VS Code extensions menu.
- Open Command Pallette via Ctrl-Shift-P
- Select "Remote-SSH: Connect to Host...", add `user@node` in the popup window, `user` being your account on the remote machine and `node` being the name or IP of the remote machine.
- A new window opens up, setting up VS Code for the remote node, this takes a few minutes
- Select "Linux" as platform
- You will be asked if you trust the remote node
- After the installation is done, open the project folder (VS Code will display the folders on the remote machine).
- When compiling, debugging, it may be necessary to (re-) install some of the extensions, C++, CMake, ... "Install in SSH: <node>" to have it available.
- As a shortcut, switch to the "Remote Explorer" View in VS Code, select "Remotes (Tunnels/SSH)" in the top combo box, use the "+" icon to add a new remote host to connect to, and add the ssh command, e.g. `ssh uer@node`. The configuration is stored and can be actrivated in the future from the Remotes View.

See also:
- https://code.visualstudio.com/docs/remote/ssh
- https://learn.microsoft.com/en-us/training/modules/develop-on-remote-machine/

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