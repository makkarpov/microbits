# ub_crypto

Small collection of cryptographic primitives, optimized mostly for minimal code size. Contains the following primitives:

* **AES** block cipher: ECB mode (encryption only) and CTR mode (encryption and decryption)
* **ChaCha20** stream cipher: encryption and decryption
* Cryptographically secure random number generator implemented with ChaCha20 primitive
* **SHA2**: SHA-256 and SHA-512
* **HMAC** over arbitrary hash function
* **SHA3**: SHA3 (any output length) and SHAKE (128 and 256 variants)
* **KMAC** with 128 and 256 bit variants
* **Ed25519** and **Ed448** digital signature schemes
* **X25519** and **X448** key exchange protocols

# Resource usage

| Test                         | Code | Stack |  Cycles   | Time  |
|:-----------------------------|:----:|:-----:|:---------:|:-----:|
| crypto_eddsa_verify_25519_Os | 4760 | 1560  | 19019006  | 0.594 |
| crypto_eddsa_verify_448_Os   | 3756 | 1800  | 175376277 | 5.48  |
| crypto_eddsa_sign_25519_Os   | 4624 | 1592  | 10714407  | 0.334 |
| crypto_eddsa_sign_448_Os     | 4200 | 1816  | 94652549  | 2.957 |
| crypto_eddh_25519_Os         | 1264 |  496  |  5980474  | 0.186 |
| crypto_eddh_448_Os           | 1380 |  896  | 45028704  | 1.407 |
| crypto_sha2_sha256_Os        | 1036 |  472  |   77130   | 0.002 |
| crypto_sha2_sha512_Os        | 2040 | 1032  |  149851   | 0.004 |
| crypto_sha3_shake256_Os      | 1016 |  360  |  712751   | 0.022 |
| crypto_sha3_sha256_Os        | 960  |  352  |  625282   | 0.019 |
| crypto_aes_Os                | 1312 |  368  |  984074   | 0.03  |
| crypto_chacha20_Os           | 564  |  224  |   73317   | 0.002 |

* Measurements were taken on STM32H563ZI (Cortex-M33) chip running at default frequency of 32 MHz.
* Hashes and ciphers are measured with 1024-byte payloads

# Acknowledgements

Curve25519 and Ed25519 code is based on "[Curve25519 and Ed25519 for low-memory systems](https://www.dlbeer.co.nz/oss/c25519.html)" 
implementation by Daniel Beer. Implementation has been adapted to run on 32-bit targets and perform most operations on 
32-bit words instead of bytes. Several optimizations were made to reduce flash footprint at cost of a small runtime overhead.
