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

| Test               | Opt | Code | Stack |  Cycles   | Time  |
|:-------------------|-----|:----:|:-----:|:---------:|:-----:|
| eddsa_verify_25519 | -Os | 4728 | 1560  | 18080915  | 0.565 |
| eddsa_verify_25519 | -O2 | 5440 | 1560  | 17111902  | 0.534 |
| eddsa_verify_448   | -Os | 3804 | 1784  | 162999144 | 5.093 |
| eddsa_verify_448   | -O2 | 4488 | 1824  | 141671579 | 4.427 |
| eddsa_sign_25519   | -Os | 4592 | 1592  | 10215362  | 0.319 |
| eddsa_sign_25519   | -O2 | 5352 | 1584  |  9684501  | 0.302 |
| eddsa_sign_448     | -Os | 4248 | 1800  | 88096565  | 2.753 |
| eddsa_sign_448     | -O2 | 5096 | 1832  | 77147226  | 2.410 |
| eddh_25519         | -Os | 1248 |  496  |  5555543  | 0.173 |
| eddh_25519         | -O2 | 1568 |  520  |  5293259  | 0.165 |
| eddh_448           | -Os | 1360 |  888  | 41670564  | 1.302 |
| eddh_448           | -O2 | 1752 |  920  | 36656370  | 1.145 |
| sha2_sha256        | -Os | 1036 |  472  |   76218   | 0.002 |
| sha2_sha256        | -O2 | 1124 |  472  |   70438   | 0.002 |
| sha2_sha512        | -Os | 2040 | 1032  |  148876   | 0.004 |
| sha2_sha512        | -O2 | 2192 | 1024  |  134750   | 0.004 |
| sha3_shake256      | -Os | 1100 |  352  |  701393   | 0.021 |
| sha3_shake256      | -O2 | 1216 |  360  |  592023   | 0.018 |
| sha3_sha256        | -Os | 960  |  352  |  625485   | 0.019 |
| sha3_sha256        | -O2 | 1040 |  360  |  527369   | 0.016 |
| aes                | -Os | 1312 |  368  |  983957   | 0.030 |
| aes                | -O2 | 1464 |  384  |  871050   | 0.027 |
| chacha20           | -Os | 564  |  224  |   73317   | 0.002 |
| chacha20           | -O2 | 636  |  216  |   63841   | 0.001 |

* Measurements were taken on STM32H563ZI (Cortex-M33) chip running at default frequency of 32 MHz.
* Hashes and ciphers are measured with 1024-byte payloads

# Acknowledgements

Curve25519 and Ed25519 code is based on "[Curve25519 and Ed25519 for low-memory systems](https://www.dlbeer.co.nz/oss/c25519.html)" 
implementation by Daniel Beer. Implementation has been adapted to run on 32-bit targets and perform most operations on 
32-bit words instead of bytes. Several optimizations were made to reduce flash footprint at cost of a small runtime overhead.
