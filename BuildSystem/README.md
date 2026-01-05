# Build System

## Build local version of openssl for PKCS support

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

## Build PKCS provider for openssl

Checkout pkcs11-provider.git provider, build, install it to local folder. A few warnings regarding runtime dependencies appear, they can be ignored.
```bash
git clone https://github.com/latchset/pkcs11-provider.git && \
pushd pkcs11-provider
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

## Build libp11

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
## Setup the Build System

### Upfront Checks

Before executing any of the steps below, verify in `MES/BuildSystem/Makefile` that 
- `TOKEN_USER_PIN` is correct
- `FIRMWARE_HOST` is the name or IP of the Firmware Management Server host
- `FIRMWARE_USER` is a valid user on the Firmware Management Server host
- the crypto token is plugged in and enabled for the virtual machine

### Generate SSH Key Pair, Install on Firmware Management Server

- cd into `MES/BuildSystem`
- run `make sshkey`. This will produce a key pair on the token and `id_rsa.pub` in the current folder
- run `make install_sshkey` to install `id_rsa.pub` on the server
- run `make agent_sshkey` to let the SSH agent provide the token PIN to ssh/scp (reduces times to enter PIN on command line)
- to test whether the ssh key works, run `make ssh_with_key`. This should ask for the PIN of the token, then open an ssh on the `FIRMWARE_HOST`.

### Generate keys and certificates, signing-tool, Install on Firmware Management Server

- cd into `MES/BuildSystem`
- run `make` to build
    - the certificate and keys for CA root
    - the Build certificate (signed by CA root) and keys
    - the Mgmnt certificate (signed by CA root) and keys
    - the `signing-tool` for signing the firmware update packages with the private key of the Build certificate
- run `make test` to test the `signing-tool` works properly. The first signing attempt must pass, the second one must fail
- if anything went wrong, do a `make clean && make`, which will recreate all certificates and keys
- run `make install_certs`, which will copy the necessary certificates and keys on the Firmware Management Server in `FIRMWARE_USER@FIRMWARE_HOST:~/FWManager`

### Deploy Firmware Update to Firmware Management Server

- cd into `MES/BuildSystem`
- adapt `MSG` and `VERSION` in `MES/BuildSystem/FWUpdate/Makefile`
- run `make firmware` to create a firmware update package in `MES/BuildSystem/FWUpdate/update.tgz`
- run `make install_firmware` to deliver the firmware to the Firmware Management Server

run `/signAndDeliver.sh FWUpdate/update.tgz firmware@<firmware-management-host>`, which will sign `update.tgz`, and send the update package with the signature file to
the Firmware Management Server.
