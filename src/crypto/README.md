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

TODO: Measurements

# Acknowledgements

Curve25519 and Ed25519 code is based on "[Curve25519 and Ed25519 for low-memory systems](https://www.dlbeer.co.nz/oss/c25519.html)" 
implementation by Daniel Beer. Implementation has been adapted to run on 32-bit targets and perform most operations on 
32-bit words instead of bytes. Several optimizations were made to reduce flash footprint at cost of a small runtime overhead.
