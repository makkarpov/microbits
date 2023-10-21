#include <ub/crypto/edwards.hpp>

#include <ub/crypto/sha2.hpp>

#include "f25519.hpp"
#include "ed25519.hpp"
#include "fprime8.hpp"

// Planned customization option - create non-standard Ed25519 variants with different hashes
// SHA-512 is way too big for some applications like bootloaders, with its giant round
#if !defined(ED25519_HASH)
#define ED25519_HASH            ub::crypto::sha512
#endif

using namespace ub::crypto;
using namespace ub::crypto::impl;

struct ed25519_msg {
    const uint8_t *message;
    size_t        length;
    const uint8_t *domain;
    size_t        domainLength;
};

struct ed25519_sign_ctx {
    uint256_t     s;
    uint8_t       prefix[32];
    ed25519_msg   m;

    inline ed25519_sign_ctx() {} // NOLINT(*)
};

struct ed25519_verify_ctx {
    ed25519_msg   m;
    const uint8_t *key;
    const uint8_t *sig;

    inline ed25519_verify_ctx() {} // NOLINT(*)
};

static constexpr size_t PH_DOMAIN_LEN = 34;
static constexpr uint8_t ph_domain[PH_DOMAIN_LEN] {
    // "SigEd25519 no Ed25519 collisions", followed by ph_flag=1, followed by len(ctx)=0
    0x53, 0x69, 0x67, 0x45, 0x64, 0x32, 0x35, 0x35, 0x31, 0x39, 0x20, 0x6E, 0x6F, 0x20, 0x45, 0x64,
    0x32, 0x35, 0x35, 0x31, 0x39, 0x20, 0x63, 0x6F, 0x6C, 0x6C, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x73,
    0x01, 0x00
};

static void ed25519_expand_key(ed25519_sign_ctx &ctx, const uint8_t *key) {
    ED25519_HASH hash;
    hash.update(key, ed25519::KEY_LENGTH);

    uint8_t digest[ED25519_HASH::OUTPUT];
    hash.finish(digest);

    digest[0] &= 0xF8;
    digest[31] = (digest[31] & 0x7F) | 0x40;

    Fp8::load(ctx.s.u8, digest, ed25519::KEY_LENGTH, C25519_ORDER);
    memcpy(ctx.prefix, digest + ed25519::KEY_LENGTH, ed25519::KEY_LENGTH);

    secureZero(digest, sizeof(digest));
}

static void ed25519_derive_r(const ed25519_sign_ctx &ctx, uint256_t &r)
{
    ED25519_HASH hash;
    hash.update(ctx.m.domain, ctx.m.domainLength);
    hash.update(ctx.prefix, 32);
    hash.update(ctx.m.message, ctx.m.length);

    uint8_t digest[ED25519_HASH::OUTPUT];
    hash.finish(digest);

    Fp8::load(r.u8, digest, ED25519_HASH::OUTPUT, C25519_ORDER);

    secureZero(digest, sizeof(digest));
}

static void ed25519_compute_R(uint256_t &r, uint8_t *signature) {
    ed25519_pt b, R;

    b.loadBase();
    ED25519::mul(R, b, r);

    R.store(signature);

    b.destroy();
}

static void ed25519_compute_k(const ed25519_msg &m, uint256_t &k, const uint8_t *publicKey, const uint8_t *R) {
    ED25519_HASH hash;
    hash.update(m.domain, m.domainLength);
    hash.update(R, ed25519::KEY_LENGTH);
    hash.update(publicKey, ed25519::KEY_LENGTH);
    hash.update(m.message, m.length);

    uint8_t digest[ED25519_HASH::OUTPUT];
    hash.finish(digest);

    Fp8::load(k.u8, digest, ED25519_HASH::OUTPUT, C25519_ORDER);
}

static void ed25519_sign_impl(ed25519_sign_ctx &ctx, const uint8_t *key, uint8_t *signature) {
    ed25519_expand_key(ctx, key);

    uint256_t r;
    ed25519_derive_r(ctx, r);
    ed25519_compute_R(r, signature);

    uint256_t k;
    ed25519_compute_k(ctx.m, k, key + ed25519::KEY_LENGTH, signature);

    uint256_t S;
    Fp8::mul(S.u8, ctx.s.u8, k.u8, C25519_ORDER);
    Fp8::add(S.u8, r.u8, C25519_ORDER);

    r.destroy();

    memcpy(signature + ed25519::KEY_LENGTH, S.u8, uint256_t::N_U8);
    secureZero(&ctx, sizeof(ctx));
}

// compute R + kA part of the signature
static bool ed25519_verify_compute_rhs(const ed25519_verify_ctx &ctx, ed25519_pt &r) {
    ed25519_pt t;

    if (!t.load(ctx.key)) {
        return false;
    }

    uint256_t k;
    ed25519_compute_k(ctx.m, k, ctx.key, ctx.sig);

    ED25519::mul(r, t, k);

    if (!t.load(ctx.sig)) {
        return false;
    }

    ED25519::add(r, r, t);
    return true;
}

// compute sB part of the signature
static bool ed25519_verify_compute_lhs(const ed25519_verify_ctx &ctx, ed25519_pt &r) {
    uint256_t S;

    Fp8::load(S.u8, ctx.sig + ed25519::KEY_LENGTH, ed25519::KEY_LENGTH, C25519_ORDER);
    if (std::memcmp(S.u8, ctx.sig + ed25519::KEY_LENGTH, ed25519::KEY_LENGTH) != 0) {
        // S must be less than L, so load must not perform any modular reduction here.
        // Modular reduction simply means that signature is invalid because S is out of range.
        return false;
    }

    ed25519_pt t;
    t.loadBase();

    ED25519::mul(r, t, S);
    return true;
}

// verification is a public procedure (no secrets are used), so there is no need to have strict timing invariance here
// attacker is more than able to redo all these computations without even looking at the device
static bool ed25519_verify_impl(ed25519_verify_ctx &ctx) {
    ed25519_pt rhs;
    if (!ed25519_verify_compute_rhs(ctx, rhs)) {
        return false;
    }

    ed25519_pt lhs;
    if (!ed25519_verify_compute_lhs(ctx, lhs)) {
        return false;
    }

    return lhs.equals(rhs);
}

static void ed25519_load_pure(ed25519_msg &msg, const uint8_t *message, size_t length) {
    msg.domain = nullptr;
    msg.domainLength = 0;
    msg.message = message;
    msg.length = length;
}

static void ed25519_load_ph(ed25519_msg &msg, const uint8_t *hash) {
    msg.domain = ph_domain;
    msg.domainLength = PH_DOMAIN_LEN;
    msg.message = hash;
    msg.length = ED25519_HASH::OUTPUT;
}

void ed25519::toPublic(uint8_t *publicKey, const uint8_t *privateKey) {
    ed25519_sign_ctx ctx;
    ed25519_expand_key(ctx, privateKey);

    ed25519_pt b, A;
    b.loadBase();

    ED25519::mul(A, b, ctx.s);

    A.store(publicKey);
}

void ed25519::sign(const uint8_t *key, uint8_t *signature, const uint8_t *message, size_t length) {
    ed25519_sign_ctx ctx;
    ed25519_load_pure(ctx.m, message, length);
    ed25519_sign_impl(ctx, key, signature);
}

void ed25519::signHash(const uint8_t *key, uint8_t *signature, const uint8_t *hash) {
    ed25519_sign_ctx ctx;
    ed25519_load_ph(ctx.m, hash);
    ed25519_sign_impl(ctx, key, signature);
}

bool ed25519::verify(const uint8_t *key, const uint8_t *signature, const uint8_t *message, size_t length) {
    ed25519_verify_ctx ctx;

    ctx.key = key;
    ctx.sig = signature;
    ed25519_load_pure(ctx.m, message, length);

    return ed25519_verify_impl(ctx);
}

bool ed25519::verifyHash(const uint8_t *key, const uint8_t *signature, const uint8_t *hash) {
    ed25519_verify_ctx ctx;

    ctx.key = key;
    ctx.sig = signature;
    ed25519_load_ph(ctx.m, hash);

    return ed25519_verify_impl(ctx);
}
