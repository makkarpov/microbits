from typing import NamedTuple, Callable
from Crypto.Cipher import AES, ChaCha20
from Crypto.Util import Counter

from devtest.runner import TestRunner
from testgen.utils import random_bytes


class CipherDefn(NamedTuple):
    key_length: int
    nonce_length: int
    encrypt: Callable[[bytes, bytes, bytes], bytes]  # (key, nonce, message) -> ciphertext
    fixed_key_length: bool


def aes_encrypt(key: bytes, nonce: bytes, message: bytes) -> bytes:
    counter = int.from_bytes(nonce, 'big')
    return AES.new(key, AES.MODE_CTR, counter=Counter.new(128, initial_value=counter)).encrypt(message)


def chacha20_encrypt(key: bytes, nonce: bytes, message: bytes) -> bytes:
    return ChaCha20.new(key=key, nonce=nonce).encrypt(message)


class TestRunnerImpl(TestRunner):
    def run(self):
        env = self._env

        cipher_name = self._args[0]
        match cipher_name:
            case 'aes': cipher = CipherDefn(32, 16, aes_encrypt, False)
            case 'chacha20': cipher = CipherDefn(32, 12, chacha20_encrypt, True)
            case _: raise RuntimeError('unknown cipher: %s' % cipher_name)

        key = random_bytes(cipher.key_length, f'dev_cipher_key_{cipher_name}')
        nonce = random_bytes(cipher.nonce_length, f'dev_cipher_nonce_{cipher_name}')
        message = random_bytes(1024, f'dev_cipher_msg_{cipher_name}')

        env.write('key', key)

        if not cipher.fixed_key_length:
            env.write('keyLength', len(key))

        env.write('nonce', nonce)
        env.write('message', message)
        env.write('messageLength', len(message))

        env.run()

        r_message = env.read('message', len(message))
        c_message = cipher.encrypt(key, nonce, message)

        if r_message != c_message:
            raise RuntimeError('device encryption failed')
