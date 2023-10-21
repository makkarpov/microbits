#include <ub/crypto/edwards.hpp>

#include <ub/crypto/sha3.hpp>

#include "bigint.hpp"
#include "fprime8.hpp"
#include "ed448.hpp"

using namespace ub::crypto;
using namespace ub::crypto::impl;

struct ed448_msg {
    const uint8_t *message;
    size_t        length;
    const uint8_t *domain;
    size_t        domainLength;
};

struct ed448_sign_ctx {
    uint448_t   s;
    uint8_t     prefix[ed448::KEY_LENGTH];
    ed448_msg   m;

    explicit ed448_sign_ctx() {} // NOLINT(*)
};

struct ed448_verify_ctx {
    ed448_msg     m;
    const uint8_t *key;
    const uint8_t *sig;

    inline ed448_verify_ctx() {} // NOLINT(*)
};

static constexpr size_t HASH_LEN = 2 * ed448::KEY_LENGTH;
static constexpr size_t PH_DOMAIN_LEN = 10;
static constexpr uint8_t ph_domain[PH_DOMAIN_LEN] {
        // "SigEd448", followed by ph_flag=1, followed by len(ctx)=0
        0x53, 0x69, 0x67, 0x45, 0x64, 0x34, 0x34, 0x38, 0x01, 0x00
};

static void ed448_expand_key(ed448_sign_ctx &ctx, const uint8_t *key) {
    shake hash(shake::FN_SHAKE256);
    hash.update(key, ed448::KEY_LENGTH);

    uint8_t digest[HASH_LEN];
    hash.generate(digest, HASH_LEN);

    digest[0]  &= 0xFC;
    digest[55] |= 0x80;

    Fp8::load(ctx.s.u8, digest, ed448::KEY_LENGTH - 1, C448_ORDER);
    std::memcpy(ctx.prefix, digest + ed448::KEY_LENGTH, ed448::KEY_LENGTH);

    secureZero(digest, sizeof(digest));
}

static void ed448_derive_r(const ed448_sign_ctx &ctx, uint448_t &r) {
    shake hash(shake::FN_SHAKE256);
    hash.update(ctx.m.domain, ctx.m.domainLength);
    hash.update(ctx.prefix, ed448::KEY_LENGTH);
    hash.update(ctx.m.message, ctx.m.length);

    uint8_t digest[HASH_LEN];
    hash.generate(digest, HASH_LEN);

    Fp8::load(r.u8, digest, HASH_LEN, C448_ORDER);

    secureZero(digest, HASH_LEN);
}

static void ed448_compute_R(uint448_t &r, uint8_t *signature) {
    ed448_pt B, R;

    B.loadBase();
    ED448::mul(R, B, r);

    R.store(signature);
}

static void ed448_compute_k(const ed448_msg &m, uint448_t &k, const uint8_t *publicKey, const uint8_t *R) {
    shake hash(shake::FN_SHAKE256);

    hash.update(m.domain, m.domainLength);
    hash.update(R, ed448::KEY_LENGTH);
    hash.update(publicKey, ed448::KEY_LENGTH);
    hash.update(m.message, m.length);

    uint8_t digest[HASH_LEN];
    hash.generate(digest, HASH_LEN);

    Fp8::load(k.u8, digest, HASH_LEN, C448_ORDER);
}

static void ed448_sign_impl(ed448_sign_ctx &ctx, const uint8_t *key, uint8_t *signature) {
    ed448_expand_key(ctx, key);

    uint448_t r;
    ed448_derive_r(ctx, r);

    ed448_compute_R(r, signature);

    uint448_t k;
    ed448_compute_k(ctx.m, k, key + ed448::KEY_LENGTH, signature);

    uint448_t S;
    Fp8::mul(S.u8, ctx.s.u8, k.u8, C448_ORDER);
    Fp8::add(S.u8, r.u8, C448_ORDER);

    r.destroy();

    std::memcpy(signature + ed448::KEY_LENGTH, S.u8, uint448_t::N_U8);
    signature[2 * ed448::KEY_LENGTH - 1] = 0x00;

    secureZero(&ctx, sizeof(ctx));
}

static bool ed448_verify_compute_rhs(ed448_verify_ctx &ctx, ed448_pt &r) {
    ed448_pt t;
    if (!t.load(ctx.key)) {
        return false;
    }

    uint448_t k;
    ed448_compute_k(ctx.m, k, ctx.key, ctx.sig);

    ED448::mul(r, t, k);

    if (!t.load(ctx.sig)) {
        return false;
    }

    ED448::add(r, r, t);
    return true;
}

static bool ed448_verify_compute_lhs(ed448_verify_ctx &ctx, ed448_pt &r) {
    uint448_t S;

    if ((ctx.sig[ed448::KEY_LENGTH - 1] & 0x7F) != 0) {
        return false;
    }

    Fp8::load(S.u8, ctx.sig + ed448::KEY_LENGTH, ed448::KEY_LENGTH, C448_ORDER);
    if (std::memcmp(S.u8, ctx.sig + ed448::KEY_LENGTH, uint448_t::N_U8) != 0) {
        // S must be less than L, so load must not perform any modular reduction here.
        // Modular reduction simply means that signature is invalid because S is out of range.
        return false;
    }

    ed448_pt t;
    t.loadBase();

    ED448::mul(r, t, S);
    return true;
}

static bool ed448_verify_impl(ed448_verify_ctx &ctx) {
    ed448_pt rhs;
    if (!ed448_verify_compute_rhs(ctx, rhs)) {
        return false;
    }

    ed448_pt lhs;
    if (!ed448_verify_compute_lhs(ctx, lhs)) {
        return false;
    }

    return lhs.equals(rhs);
}

static void ed448_load_pure(ed448_msg &m, const uint8_t *message, size_t length) {
    m.message = message;
    m.length = length;
    m.domain = nullptr;
    m.domainLength = 0;
}

void ed448::toPublic(uint8_t *publicKey, const uint8_t *privateKey) {
    ed448_sign_ctx ctx;
    ed448_expand_key(ctx, privateKey);

    ed448_pt B, A;

    B.loadBase();
    ED448::mul(A, B, ctx.s);

    secureZero(&ctx, sizeof(ctx));
    A.store(publicKey);
}

void ed448::sign(const uint8_t *key, uint8_t *signature, const uint8_t *message, size_t length) {
    ed448_sign_ctx ctx;
    ed448_load_pure(ctx.m, message, length);
    ed448_sign_impl(ctx, key, signature);
}

static void ed448_load_ph(ed448_msg &m, const uint8_t *hash) {
    m.message = hash;
    m.length = ed448::HASH_LENGTH;
    m.domain = ph_domain;
    m.domainLength = PH_DOMAIN_LEN;
}

void ed448::signHash(const uint8_t *key, uint8_t *signature, const uint8_t *hash) {
    ed448_sign_ctx ctx;
    ed448_load_ph(ctx.m, hash);
    ed448_sign_impl(ctx, key, signature);
}

bool ed448::verify(const uint8_t *key, const uint8_t *signature, const uint8_t *message, size_t length) {
    ed448_verify_ctx ctx;
    ed448_load_pure(ctx.m, message, length);
    ctx.key = key;
    ctx.sig = signature;
    return ed448_verify_impl(ctx);
}

bool ed448::verifyHash(const uint8_t *key, const uint8_t *signature, const uint8_t *hash) {
    ed448_verify_ctx ctx;
    ed448_load_ph(ctx.m, hash);
    ctx.key = key;
    ctx.sig = signature;
    return ed448_verify_impl(ctx);
}
