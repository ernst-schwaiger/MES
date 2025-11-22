# PKCS Support for Openssl

## Build local version of openssl for PKCS support

Install dependencies

```bash
sudo apt install p11-kit libp11-kit-dev pkg-config ninja-build gcc make meson opensc
```

Checkout 3.0.x version of openssl, e.g. 3.0.14 (avoid versions 4.x!)
```bash
git clone https://github.com/openssl/openssl.git
cd openssl
git checkout openssl-3.0.14
```

Configure installation into user home folder, compile, install
```bash
./Configure --prefix=$HOME/openssl-local --openssldir=$HOME/openssl-local/ssl
make -sj
make install
```

Append the following entries to `$HOME/.bashrc`. `openssl` will then run the compiled openssl instead of the installed one. Remove these lines to run the installed openssl version again.

```bash
export LD_LIBRARY_PATH="$HOME/openssl-local/lib64:$LD_LIBRARY_PATH"
export PATH="$HOME/openssl-local/bin:$PATH"
export PKG_CONFIG_PATH="$HOME/openssl-local/lib64/pkgconfig:$PKG_CONFIG_PATH"
```

Open a new terminal so that the settings are taken over, test with `openssl version`. It must produce the checked out version, e.g. 3.0.14.

## Build PKCS provider for openssl

Checkout pkcs11-provider.git provider

```bash
git clone https://github.com/latchset/pkcs11-provider.git
```

Build provider, install it to local folder. A few warnings regarding runtime dependencies appear, they can be ignored.
```bash
meson setup build --prefix=$HOME/pkcs11-local
ninja -C build
ninja -C build install
```

`$HOME/openssl-local/lib64/ossl-modules` should now contain the shared object of the PKCS provider, `pkcs11.so`.

Find the opensc-pkcs11 driver module that was installed with the dependencies in the first step:
```bash
find /usr/lib -name "opensc-pkcs11.so" -type f
/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so
```
If the driver is installed somewhere else, use that path subsequently

Add the `pkcs11`, `pkcs11_sect` entries to $HOME/openssl-local/ssl/openssl.cnf. Replace `/home/ernst/openssl-local` with the path to your local openssl installation:

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
```

`openssl list -providers -provider pkcs11` should now list the provider for pkcs11.

## Create an RSA private/public key pair using 

Ensure the crypto token is plugged in, and that in the VM the "Devices/USB", an "FS USB Token" shows up and is checked. Generate the key pair via `pkcs11-tool`:

```bash
pkcs11-tool --module /usr/lib/x86_64-linux-gnu/opensc-pkcs11.so \
            --slot 0 \
            --login \
            --pin "123456" \
            --keypairgen \
            --key-type rsa:2048 \
            --id 01 \
            --label "MES_Key"
```

## Generate a certificate signing request for PKCS key pair

Adapt the `-subj` field to what the cert shall contain

```bash
openssl req -new -provider pkcs11 -provider base -propquery '?provider=pkcs11' \
    -key "pkcs11:object=MES_Key;type=private" \
    -subj "/CN=Test/O=Org/C=AT"     \
    -out myreq.csr
```

Generate self-signed root cert
```bash
openssl genrsa -out rootCA.key 2048
openssl req -x509 -new -nodes \
    -key rootCA.key \
    -sha256 -days 365 \
    -subj "/CN=MyRootCA/O=MyOrg/C=AT" \
    -out rootCA.crt
```

Sign certificate with root cert:
```bash
openssl x509 -req \
    -in myreq.csr \
    -CA rootCA.crt -CAkey rootCA.key \
    -CAcreateserial \
    -out mes_key.crt \
    -days 365 -sha256
```

Convert certificate to der/binary format
```bash
openssl x509 -in mes_key.crt -outform der -out mes_key.der
```

Write certificate back to security token

```bash
pkcs11-tool --module /usr/lib/x86_64-linux-gnu/opensc-pkcs11.so \
            --slot 0 \
            --login \
            --pin "123456" \
            --write-object mes_key.der \
            --type cert \
            --id 01 \
            --label "MES_Key_Cert"
```


## Build libp11

Download released tarball from github, e.g. [libp11-0.4.16.tar.gz](https://github.com/OpenSC/libp11/releases/download/libp11-0.4.16/libp11-0.4.16.tar.gz), extract it, cd into the folder and configure it with the locally compiled openssl version, install it below the $HOME folder again.

```bash
LDFLAGS=-L$HOME/openssl-local/lib64 CFLAGS=-I$HOME/openssl-local/include ./configure --prefix=$HOME/libp11-local
make -sj && make install
```

