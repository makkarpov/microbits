# ub_crypto

Small collection of cryptographic primitives, optimized for minimal code size. Contains the following primitives:

* AES (encryption only)
* SHA2 (SHA-256 and SHA-512)
* Ed25519 and Ed448

# Resource usage

TODO: Measurements

# Acknowledgements

Curve25519 and Ed25519 code is based on "[Curve25519 and Ed25519 for low-memory systems](https://www.dlbeer.co.nz/oss/c25519.html)" 
implementation by Daniel Beer. Implementation has been adapted to run on 32-bit targets and perform most operations on 
32-bit words instead of bytes. Several optimizations were made to reduce flash footprint at cost of a small runtime overhead.
