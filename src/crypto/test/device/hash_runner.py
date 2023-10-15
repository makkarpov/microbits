from devtest.runner import TestRunner
from testgen.utils import random_bytes, random_number
import hashlib


class TestRunnerImpl(TestRunner):
    def run(self):
        env = self._env
        algo = self._args[0]

        if not hasattr(hashlib, algo):
            raise RuntimeError('algorithm not found: %s' % algo)

        digest_len = getattr(hashlib, algo)().digest_size
        var_length = digest_len == 0

        message = random_bytes(1024, 'device:%s' % algo)
        env.write('message', message)
        env.write('messageLen', len(message))

        if var_length:
            digest_len = 1 + random_number(255, 'device:digest_len:' + algo)
            env.write('digestLen', digest_len)

        env.run()

        if var_length:
            v_digest = getattr(hashlib, algo)(message).digest(digest_len)
            r_digest = env.read('digest', digest_len)
        else:
            v_digest = getattr(hashlib, algo)(message).digest()
            r_digest = env.read('digest')

        if v_digest != r_digest:
            print('hashed message:  %s' % message.hex())
            print('expected digest: %s' % v_digest.hex())
            print('actual digest:   %s' % r_digest.hex())
            raise RuntimeError('digest mismatch')
