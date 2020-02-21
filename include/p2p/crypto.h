#pragma once

#include <multiformats\multibase.h>
#include <multiformats\multihash.h>

namespace p2p { namespace crypto {

    namespace mf = multiformats;

    using bigint_t = mf::buffer_t;
    using mf::hash_t;
    using mf::buffer_t;
    using mf::bufferview_t;
    using mf::digest_buffer;



    class rsa_public_key {
    public:
        bigint_t n, e;

    public:
        // Create an empty key
        rsa_public_key() {}

        // Initialize a RSA public key with
        // - n: the modulus
        // - e: the public exponent
        rsa_public_key(const bigint_t& n, const bigint_t& e) : n(n), e(e)
        {}

        // Convert to/from Json Web Key
        std::string           to_jwk() const;
        static rsa_public_key from_jwk(const std::string& jwk);

        // Convert to/from PKCS (ASN1 DER encoded)
        buffer_t              to_pkcs() const;
        static rsa_public_key from_pkcs(bufferview_t pkcs);

        // Convert to/from protobuf (match go-ipfs formatting)
        buffer_t              to_protobuf() const;
        static rsa_public_key from_protobuf(bufferview_t pkcs);

        inline bool empty()       const { return n.empty(); }
    };

    inline bool operator==(const rsa_public_key& _Left, const rsa_public_key& _Right) {
        return (_Left.e == _Right.e)
            && (_Left.n == _Right.n);
    }



    class rsa_private_key {
    public:

        bigint_t n, e, d, p, q, dp, dq, qi;

    public:
        // Create an empty key
        rsa_private_key() {}

        // Initialize a RSA private key with:
        // - n: modulus
        // - e: public exponent
        // - d: private exponent
        // - p: first prime factor
        // - q: second prime factor
        // - dp: d mod p
        // - dq: d mod q
        // - qi: q^-1 mod p
        rsa_private_key(const bigint_t& n, const bigint_t& e, const bigint_t& d, const bigint_t& p, const bigint_t& q, const bigint_t& dp, const bigint_t& dq, const bigint_t& qi)
            : n(n), e(e), d(d), p(p), q(q), dp(dp), dq(dq), qi(qi)
        {}

        // Convert to/from Json Web Key
        std::string            to_jwk() const;
        static rsa_private_key from_jwk(const std::string& jwk);

        // Convert to/from PKCS (ASN1 DER encoded)
        buffer_t               to_pkcs() const;
        static rsa_private_key from_pkcs(bufferview_t pkcs);

        // Convert to/from protobuf (match go-ipfs formatting)
        buffer_t               to_protobuf() const;
        static rsa_private_key from_protobuf(bufferview_t pkcs);


        inline rsa_public_key public_key() const { return { n, e }; }
        inline bool           empty()      const { return n.empty(); }
    };

    inline bool operator==(const rsa_private_key& _Left, const rsa_private_key& _Right) {
        return (_Left.e == _Right.e)
            && (_Left.n == _Right.n)
            && (_Left.d == _Right.d)
            && (_Left.p == _Right.p)
            && (_Left.q == _Right.q)
            && (_Left.dp == _Right.dp)
            && (_Left.dq == _Right.dq)
            && (_Left.qi == _Right.qi);
    }



    // Generate RSA public/private key pair
    const uint32_t default_privkey_bitsize = 2048;
    rsa_private_key generate_keypair(uint32_t bits = default_privkey_bitsize);

}}