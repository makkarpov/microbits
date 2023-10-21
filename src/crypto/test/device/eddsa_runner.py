from devtest.runner import TestRunner


class TestRunnerImpl(TestRunner):
    def run(self):
        env = self._env
        msg = bytes.fromhex('00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff')

        if self._args[1] == '25519':
            prv = bytes.fromhex('0000000000000000000000000000000000000000000000000000000000000000')
            pub = bytes.fromhex('3b6a27bcceb6a42d62a3a8d02a6f0d73653215771de243a63ac048a18b59da29')
            sig = bytes.fromhex('b715b42bcdf6a3755d83f500103441557410920c276efb3102752f36e301ccde'
                                'c2e5015ebdd6595a1844518a69b85f156db8ed9792d85f483f5c779ae2b90b0f')
        elif self._args[1] == '448':
            prv = bytes.fromhex('0000000000000000000000000000000000000000000000000000000000000000'
                                '00000000000000000000000000000000000000000000000000')
            pub = bytes.fromhex('5b3afe03878a49b28232d4f1a442aebde109f807acef7dfd9a7f65b962fe52d6'
                                '547312cacecff04337508f9d2529a8f1669169b21c32c48000')
            sig = bytes.fromhex('85e0b7b537cbc6f377e5f341284ca59f1fa61500d3a5c8e2bc4d0a6393337335'
                                'e9c3fb106d11c5c46da64c659e7cc25dc6e64423f18041d480a5a0a04081f7f7'
                                'f37c86f2b2caf9132417e87132f9bf1064be06ee88ba981227523b11d5da5c33'
                                '395044917d125c5375b768b7441039cb3500')
        else:
            raise RuntimeError('unknown signature type: %s' % self._args[1])

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

            r_sig = env.read('signature', len(sig))
            if r_sig != sig:
                raise RuntimeError('signing failed')
