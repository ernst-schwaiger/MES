from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes

from Crypto.Protocol.KDF import PBKDF2
from Crypto.Hash import SHA512
from Crypto.Random import get_random_bytes
from Crypto.Util.Padding import pad

from Crypto.PublicKey import ECC

import argparse
import json

NONCE_SIZE = 16
TAG_SIZE = 16
KEY_LENGTH = 32
SECRET_LENGTH = 32

def parse_arguments():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--client-public-key', '-c', required=True, help='Client public key file path')
    parser.add_argument('--ephemeral-public-output', '-eo', required=True, help='Ephemeral public key file path')
    parser.add_argument('--output', '-o', default="ECIES.json", help='Session key file path')
    parser.add_argument('--encrypted-firmware-filename', '-fn', default="firmware-latest.bin", help='Filename for encrypted firmware')
    parser.add_argument('firmware', help='The path to the firmware to be encrypted')
    return parser.parse_args()

def encrypt_firmware(input_file, output_file, key):
    nonce = get_random_bytes(NONCE_SIZE)
    cipher = AES.new(key, AES.MODE_CBC, nonce)

    with open(input_file, "rb") as f:
        plaintext = f.read()

    ciphertext = cipher.encrypt(pad(plaintext, AES.block_size))

    with open(output_file, "wb") as f:
        f.write(nonce)
        f.write(ciphertext)

    return cipher.iv

def encrypt_key(encryption_key, wrapper_key):
    nonce = get_random_bytes(NONCE_SIZE)
    cipher = AES.new(wrapper_key, AES.MODE_GCM, nonce=nonce)
    ciphertext, tag = cipher.encrypt_and_digest(encryption_key)
    return nonce + tag + ciphertext

def derive_keys(password, salt):
    iterations = 200000                 
    keys = PBKDF2(password, salt, KEY_LENGTH, iterations, hmac_hash_module=SHA512)
    return keys[:KEY_LENGTH]


def generate_shared_secret_encrypt(public_cert_path, ephemeral_public_key_path):   

    ephemeral_key = ECC.generate(curve="P-256")
    ephemeral_private = ephemeral_key
    ephemeral_public = ephemeral_key.public_key()

    with open(public_cert_path, "rt") as f:
        receiver_public = ECC.import_key(f.read())

    shared_point = ephemeral_private.d * receiver_public.pointQ
    shared_secret = int(shared_point.x).to_bytes(SECRET_LENGTH, "big")

    with open(ephemeral_public_key_path, "wt") as f:
        data = ephemeral_public.export_key(format='PEM')
        f.write(data)

    return shared_secret, ephemeral_public

def generate_shared_secret_decrypt(ephemeral_public):   
    with open("./cert/private-self.key", "rt") as f:
        receiver_private = ECC.import_key(f.read())

    shared_point = receiver_private.d * ephemeral_public.pointQ
    shared_secret = int(shared_point.x).to_bytes(32, "big")

    return shared_secret

def decrypt_key(encrypted_key, ephemeral_public, salt):
    password  = generate_shared_secret_decrypt(ephemeral_public)
    enc_key = derive_keys(password, salt)

    nonce = encrypted_key[:NONCE_SIZE]
    tag = encrypted_key[NONCE_SIZE:NONCE_SIZE+TAG_SIZE]
    ciphertext = encrypted_key[NONCE_SIZE+TAG_SIZE:]

    cipher = AES.new(enc_key, AES.MODE_GCM, nonce=nonce)

    cipher.decrypt_and_verify(ciphertext, tag)

def main(args):

    # Randomly generated 256-bit secret as K_enc
    encryption_key = get_random_bytes(KEY_LENGTH)
    iv = encrypt_firmware(args.firmware, args.encrypted_firmware_filename, encryption_key)

    password, ephemeral_public = generate_shared_secret_encrypt(args.client_public_key, args.ephemeral_public_output)   

    salt = get_random_bytes(NONCE_SIZE)

    # The K_wrap for encrypting K_enc
    wrapper_key = derive_keys(password, salt)

    session_key = encrypt_key(encryption_key, wrapper_key)

    public_key = ephemeral_public.export_key(format='PEM')
    template = {
        "ephemeral-public-key": args.ephemeral_public_output,
        "salt": salt.hex(),
        "session-key": session_key.hex()
    }

    with open(args.output, 'w') as f:
        json.dump(template, f, indent=4)

    # Uncomment for testing purposes
    # decrypt_key(session_key, ephemeral_public, salt)

# TODO integrate output into script gen_manifest.py
if __name__ == "__main__":
    _args = parse_arguments()
    main(_args)