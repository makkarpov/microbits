from devtest.runner import TestRunner
from testgen.rfc7748_ref import CurveDefn, X25519, X448


class TestRunnerImpl(TestRunner):
    def run(self):
        env = self._env

        if self._args[0] == '25519':
            curve = X25519
        elif self._args[0] == '448':
            curve = X448
        else:
            raise RuntimeError('unknown curve: ' + self._args[0])

        priv1 = b'\x33' * curve.length
        priv2 = b'\x55' * curve.length

        pub1s = curve.compute_fn(curve.scalar_fn(priv1), None)
        pub1 = pub1s.to_bytes(curve.length, 'little')
        secret = curve.compute_fn(curve.scalar_fn(priv2), pub1s).to_bytes(curve.length, 'little')

        env.write('publicKey', pub1)
        env.write('privateKey', priv2)

        env.run()

        r_sig = env.read('secret')
        if r_sig != secret:
            raise RuntimeError(curve.name + ' test failed')
