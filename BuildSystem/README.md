# PKCS Support for Openssl

## Build local version of openssl for PKCS support

Install dependencies

```bash
sudo apt install p11-kit libp11-kit-dev pkg-config ninja-build gcc make meson opensc
```

Get the version of the used openssl version via `openssl version` then download the sources for exactly this version, e.g. 3.0.14 for Ubuntu 22-04 or 3.4.0 for Kali 2025-08-12 (avoid versions 4.x!), configure installation into user home folder with debugging info, compile, install

```bash
wget https://www.openssl.org/source/openssl-3.0.14.tar.gz && \
tar -xzf openssl-3.0.14.tar.gz && \
pushd openssl-3.0.14 && \
./Configure --prefix=$HOME/openssl-local --openssldir=$HOME/openssl-local/ssl debug-linux-x86_64 && \
make -sj && make install && \
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
meson setup build --prefix=$HOME/pkcs11-local && \
ninja -C build && \
ninja -C build install
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

In `$HOME/.bashrc` or `$HOME/.zshrc` adapt LD_LIBRARY_PATH so `libp11.so.3` can be found.

```bash
...
export LD_LIBRARY_PATH="$HOME/openssl-local/lib64:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$HOME/libp11-local/lib:$LD_LIBRARY_PATH"
...
```

## Generate keys and certificates, build and test the signing-tool

- cd into `MES/BuildSystem`.
- Verify in `Makefile` that the entries for `TOKEN_USER_PIN` and `LIB_P11_FOLDER`are correct.
- Ensure that the crypto token is plugged in and enabled for the virtual machine. 
- Running `make` without parameters will create the certificates and keys for CA root, Build, and Mgmnt. It will also compile the `signing-tool` which signs packages using the private Build key. At some point, the token PIN must be entered on the command line.
- `make clean` removes the certificates, keys and `signing-tool` again (error messages regarding failed key removal on token can be ignored).
- `make test` verifies that `signing-tool` works. The first signing verification must be successful, the second one has to fail.
- `make sshkey` creates an SSH key pair on the token and exports a matching `id_rsa.pub`. This key can be installed on the Firmware Management server via `ssh-copy-id -i id_rsa.pub <user>@<Firmware-Mgmnt-Server-Name-or-IP>`.
