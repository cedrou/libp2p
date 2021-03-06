#include <catch.hpp>

#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <p2p/crypto.h>

using namespace p2p::crypto;
using namespace multiformats;


TEST_CASE("RSA Keypair")
{
    auto prvkey = generate_keypair(0);
    auto pubkey = prvkey.public_key();

    REQUIRE(rsa_public_key::from_jwk(pubkey.to_jwk()) == pubkey);
    REQUIRE(rsa_public_key::from_pkcs(pubkey.to_pkcs()) == pubkey);
    REQUIRE(rsa_public_key::from_protobuf(pubkey.to_protobuf()) == pubkey);

    REQUIRE(rsa_private_key::from_jwk(prvkey.to_jwk()) == prvkey);
    REQUIRE(rsa_private_key::from_pkcs(prvkey.to_pkcs()) == prvkey);
    REQUIRE(rsa_private_key::from_protobuf(prvkey.to_protobuf()) == prvkey);
}