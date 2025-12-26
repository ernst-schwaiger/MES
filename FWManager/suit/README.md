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

## Start
```shell
python3 .\suit-tool
```

## Generate Manifest
```shell
python3 .\gen_manifest.py 0

python3 ..\bin\suit-tool create -i ..\..\out.json -o test.json -f json
```