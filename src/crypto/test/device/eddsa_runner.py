from devtest.runner import TestRunner


class TestRunnerImpl(TestRunner):
    def run(self):
        env = self._env
        prv = bytes.fromhex('0000000000000000000000000000000000000000000000000000000000000000')
        pub = bytes.fromhex('3b6a27bcceb6a42d62a3a8d02a6f0d73653215771de243a63ac048a18b59da29')
        msg = bytes.fromhex('00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff')
        sig = bytes.fromhex('b715b42bcdf6a3755d83f500103441557410920c276efb3102752f36e301ccde'
                            'c2e5015ebdd6595a1844518a69b85f156db8ed9792d85f483f5c779ae2b90b0f')

        env.write('message', msg)
        env.write('messageLen', len(msg))

        if self._args[0] == 'verify':
            env.write('key', pub)
            env.write('signature', sig)

            sig_valid = env.run(result_len=4)

            if sig_valid != 1:
                raise RuntimeError('signature verification failed')
        else:
            env.write('key', prv + pub)
            env.run()

            r_sig = env.read('signature')
            if r_sig != sig:
                raise RuntimeError('signing failed')
