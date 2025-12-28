# Setup

## Python venv
```shell
python -m venv venv
```

## Activate venv
```shell
.\venv\Scripts\Activate # on Windows
```

## Install packages for SUIT Tool
```shell
pip install cryptography
```

## Network
```shell
make network
```

## Generate Manifest
```shell
make all
```

## Generate Manifest in JSON-format
```shell
make json
```

## ECIES and Firmware encryption
```shell
# Create ECC key pair beforehand
python3 encrypt_firmware.py -c <path-to-client-public-Cert> -eo <path-to-ephemeral-public-key-to-save> <path-to-firmware-to-be-encrypted> -fn <path-of-encrypted-firmware>

# See usage for more info
```