#include <p2p/crypto.h>
#include <p2p/utils/json.h>
#include <p2p/utils/template_string.h>

#include <multiformats/uvarint.h>

#pragma warning( push )  
#pragma warning( disable : 4250 ) // 'class1' : inherits 'class2::member' via dominance 
#include <botan/rsa.h>
#include <botan/auto_rng.h>
#include <botan/asn1_oid.h>
#include <botan/der_enc.h>
#include <botan/ber_dec.h>
#include <botan\sha2_32.h>

#pragma warning ( pop )

#include <iostream>



using namespace p2p::crypto;
using namespace multiformats;



buffer_t to_buffer(const Botan::BigInt& bigint)
{
    auto b = buffer_t(bigint.encoded_size(Botan::BigInt::Binary), 0);
    bigint.binary_encode(b.data());
    return b;
}

template <class T>
buffer_t to_buffer(const Botan::secure_vector<T>& secvec)
{
    return { secvec.begin(), secvec.end() };
}


//const rsa_public_key rsa_public_key::empty{};
//const rsa_private_key rsa_private_key::empty{};


std::string rsa_public_key::to_jwk() const {
    return template_string{ R"({ "kty":"RSA", "n":"${n}", "e":"${e}" })" }
        .set("${n}", encode<base64url>(n).str())
        .set("${e}", encode<base64url>(e).str())
        ;
}

rsa_public_key rsa_public_key::from_jwk(const std::string& jwk)
{
    //TODO: use a real JSON parser

    auto kty = json::getstring(jwk, "kty");
    if (kty.empty()) throw std::invalid_argument("Invalid JWK string");
    if (kty != "RSA") throw std::invalid_argument("Not a valid RSA key");

    auto n = json::getstring(jwk, "n");
    if (n.empty()) throw std::invalid_argument("Not a valid RSA key");
    
    auto e = json::getstring(jwk, "e");
    if (e.empty()) throw std::invalid_argument("Not a valid RSA key");

    return { decode<base64url>(n), decode<base64url>(e) };
}



std::string rsa_private_key::to_jwk() const {
    return template_string{ R"({ "kty":"RSA", "n":"${n}", "e":"${e}", "d":"${d}", "p":"${p}", "q":"${q}", "dp":"${dp}", "dq":"${dq}", "qi":"${qi}", "alg":"RS256", "kid":"2011-04-29" })" }
        .set("${n}", encode<base64url>(n).str())
        .set("${e}", encode<base64url>(e).str())
        .set("${d}", encode<base64url>(d).str())
        .set("${p}", encode<base64url>(p).str())
        .set("${q}", encode<base64url>(q).str())
        .set("${dp}", encode<base64url>(dp).str())
        .set("${dq}", encode<base64url>(dq).str())
        .set("${qi}", encode<base64url>(qi).str())
        ;
}

rsa_private_key rsa_private_key::from_jwk(const std::string& jwk)
{
    //TODO: use a real JSON parser

    auto kty = json::getstring(jwk, "kty");
    if (kty.empty()) throw std::invalid_argument("Invalid JWK string");
    if (kty != "RSA") throw std::invalid_argument("Not a valid RSA key");

    auto n = json::getstring(jwk, "n");    if (n.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto e = json::getstring(jwk, "e");    if (e.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto d = json::getstring(jwk, "d");    if (d.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto p = json::getstring(jwk, "p");    if (p.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto q = json::getstring(jwk, "q");    if (q.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto dp = json::getstring(jwk, "dp");  if (dp.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto dq = json::getstring(jwk, "dq");  if (dq.empty()) throw std::invalid_argument("Not a valid RSA key");
    auto qi = json::getstring(jwk, "qi");  if (qi.empty()) throw std::invalid_argument("Not a valid RSA key");

    return {
        decode<base64url>(n),
        decode<base64url>(e),
        decode<base64url>(d),
        decode<base64url>(p),
        decode<base64url>(q),
        decode<base64url>(dp),
        decode<base64url>(dq),
        decode<base64url>(qi)
    };
}


// ASN.1 syntax for RSA public/private keys: 
//    https://www.emc.com/collateral/white-papers/h11300-pkcs-1v2-2-rsa-cryptography-standard-wp.pdf
//    https://tls.mbed.org/kb/cryptography/asn1-key-structures-in-der-and-pem
//
//  ----- PKCS#1 -----
//
//  RSAPublicKey :== SEQUENCE {
//      modulus             INTEGER,                --  n
//      publicExponent      INTEGER                 --  e
//  }
//
//  Version ::= INTEGER { two-prime(0), multi(1) }
//      (CONSTRAINED BY {-- version must be multi if otherPrimeInfos present --})
//  
//  RSAPrivateKey :: = SEQUENCE{
//      version           Version,
//      modulus           INTEGER,                 --n
//      publicExponent    INTEGER,                 --e
//      privateExponent   INTEGER,                 --d
//      prime1            INTEGER,                 --p
//      prime2            INTEGER,                 --q
//      exponent1         INTEGER,                 --d mod(p - 1)
//      exponent2         INTEGER,                 --d mod(q - 1)
//      coefficient       INTEGER,                 --(inverse of q) mod p
//      otherPrimeInfos   OtherPrimeInfos OPTIONAL
//  }
//
// ----- PKCS#8 -----
//
//  RSAAlgorithmIdentifier ::= SEQUENCE {
//      algorithm       OBJECT IDENTIFIER,          --  'RSA' = '1.2.840.113549.1.1.1'
//      none            NULL OPTIONAL
//  }
//
//  PublicKey ::= SEQUENCE {
//      algorithm           RSAAlgorithmIdentifier,
//      subjectPublicKey    BIT STRING              --  RSAPublicKey
//  }
//
//  PrivateKeyInfo ::= SEQUENCE {
//      version         Version,
//      algorithm       AlgorithmIdentifier,
//      PrivateKey      OCTET STRING
//  }



buffer_t rsa_public_key::to_pkcs() const {
    //auto pk = Botan::RSA_PublicKey{ { n.data(), n.size() },{ e.data(), e.size() } };
    //return pk.subject_public_key();

    auto aid = Botan::AlgorithmIdentifier("RSA", Botan::AlgorithmIdentifier::USE_NULL_PARAM);

    auto pk_bits = Botan::DER_Encoder()
        .start_cons(Botan::ASN1_Tag::SEQUENCE)
            .encode(Botan::BigInt(n.data(), n.size()))
            .encode(Botan::BigInt(e.data(), e.size()))
        .end_cons()
        .get_contents_unlocked();

    return Botan::DER_Encoder()
        .start_cons(Botan::ASN1_Tag::SEQUENCE)
            .encode(aid)
            .encode(pk_bits, Botan::ASN1_Tag::BIT_STRING)
        .end_cons()
        .get_contents_unlocked();


    //return Botan::DER_Encoder{}
    //    .start_cons(Botan::ASN1_Tag::SEQUENCE)                  //  PublicKey

    //        .start_cons(Botan::ASN1_Tag::SEQUENCE)              //      algorithm
    //            .encode(Botan::OID("1.2.840.113549.1.1.1"))     //          algorithm = 'RSA'
    //            .encode_null()                                  //          none: null
    //        .end_cons()

    //        //.start_cons(Botan::ASN1_Tag::BIT_STRING)            //      subjectPublicKey
    //            .start_cons(Botan::ASN1_Tag::SEQUENCE)          //          RSAPublicKey
    //                .encode(Botan::BigInt(n.data(), n.size()))  //              modulus
    //                .encode(Botan::BigInt(e.data(), e.size()))  //              exponent
    //            .end_cons()
    //        //.end_cons()

    //    .end_cons()
    //    .get_contents_unlocked();
}

rsa_public_key rsa_public_key::from_pkcs(bufferview_t pkcs)
{
    auto aid = Botan::AlgorithmIdentifier{};
    auto pk_bits = buffer_t{};

    Botan::BER_Decoder{ pkcs.data(), gsl::narrow<size_t>(pkcs.size()) }
        .start_cons(Botan::ASN1_Tag::SEQUENCE)         //  PublicKey
            .decode(aid)
            .decode(pk_bits, Botan::ASN1_Tag::BIT_STRING)
        .verify_end()
        .end_cons();

    auto pk = Botan::RSA_PublicKey{ aid, pk_bits };

    //Botan::BigInt n, e;
    //Botan::BIT_STRING;

    //Botan::BER_Decoder{ pkcs.data(), gsl::narrow<size_t>(pkcs.size()) }
    //    .start_cons(Botan::ASN1_Tag::SEQUENCE)         //  PublicKey

    //        .start_cons(Botan::ASN1_Tag::SEQUENCE)     //      algorithm
    //            .decode_and_check(Botan::OID("1.2.840.113549.1.1.1"), "Invalid RSA public key")     //          algorithm = 'RSA'
    //            .decode_null()                         //          none: null
    //        .verify_end()
    //        .end_cons()

    //        //.start_cons(Botan::ASN1_Tag::BIT_STRING)   //      subjectPublicKey
    //            .start_cons(Botan::ASN1_Tag::SEQUENCE) //          RSAPublicKey
    //                .decode(n)                         //              modulus
    //                .decode(e)                         //              exponent
    //            .verify_end()
    //            .end_cons()
    //        //.verify_end()
    //        //.end_cons()

    //    .verify_end()
    //    .end_cons();

    return {
        to_buffer(pk.get_n()),
        to_buffer(pk.get_e())
    };
}


buffer_t rsa_private_key::to_pkcs() const {
    return Botan::DER_Encoder{}
        .start_cons(Botan::ASN1_Tag::SEQUENCE)          
            .encode(static_cast<size_t>(0))
            .encode(Botan::BigInt(n.data(), n.size()))
            .encode(Botan::BigInt(e.data(), e.size()))
            .encode(Botan::BigInt(d.data(), d.size()))
            .encode(Botan::BigInt(p.data(), p.size()))
            .encode(Botan::BigInt(q.data(), q.size()))
            .encode(Botan::BigInt(dp.data(), dp.size()))
            .encode(Botan::BigInt(dq.data(), dq.size()))
            .encode(Botan::BigInt(qi.data(), qi.size()))
        .end_cons()
        .get_contents_unlocked();
}

rsa_private_key rsa_private_key::from_pkcs(bufferview_t pkcs) {
    Botan::BigInt n,e,d,p,q,dp,dq,qi;

    Botan::BER_Decoder{ pkcs.data(), gsl::narrow<size_t>(pkcs.size()) }
        .start_cons(Botan::ASN1_Tag::SEQUENCE)
            .decode_and_check<size_t>(0, "Unknown PKCS #1 key format version")
            .decode(n)
            .decode(e)
            .decode(d)
            .decode(p)
            .decode(q)
            .decode(dp)
            .decode(dq)
            .decode(qi)
        .verify_end()
        .end_cons();

    return {
        to_buffer(n),
        to_buffer(e),
        to_buffer(d),
        to_buffer(p),
        to_buffer(q),
        to_buffer(dp),
        to_buffer(dq),
        to_buffer(qi)
    };
}



// TODO: Use protobuf library
// message PublicPrivateKey {
//     required KeyType Type = 1;   --   field_number:1(0001) wire_type:varint(000) value=KeyType.RSA(00000000)
//                                  --   0000'1000 0000'0000 
//                                  --   0x08      0x00
//     required bytes Data = 2;     --   field_number:2(0010) wire_type:bytes(010) size=??
//                                  --   0001'0010 ????'????
//                                  --   0x12      0xA6 0x02
// }
template <class RsaKey>
buffer_t to_protobuf(const RsaKey& key)
{
    auto pkcs = key.to_pkcs();
    auto size = pkcs.size();

    auto b = buffer_t{ 0x08, 0x00, 0x12 };

    // encode data size size 
    while (size > 0x7F)
    {
        b.push_back(0x80 + (size & 0x7F));
        size >>= 7;
    }
    b.push_back(size & 0x7F);

    b.insert(b.end(), pkcs.begin(), pkcs.end());

    return b;
}

template <class RsaKey>
RsaKey from_protobuf(bufferview_t buffer)
{
    if (buffer.first<3>() != buffer_t{ 0x08, 0x00, 0x12 }) throw std::invalid_argument("Not a valid RSA key protobuf");
    
    auto index = 3;
    auto size = size_t{ 0 };
    auto shift = 0;
    while (buffer[index] & 0x80) {
        size += (buffer[index++] & 0x7F) << shift;
        shift += 7;
    }
    size += (buffer[index++] & 0x7F) << shift;

    if (buffer.size() != index + size) throw std::invalid_argument("Not a valid RSA key protobuf");

    return RsaKey::from_pkcs(buffer.last(size));
}


buffer_t rsa_public_key::to_protobuf() const { return ::to_protobuf(*this); }
buffer_t rsa_private_key::to_protobuf() const { return ::to_protobuf(*this); }

rsa_public_key rsa_public_key::from_protobuf(bufferview_t pkcs) { return ::from_protobuf<rsa_public_key>(pkcs); }
rsa_private_key rsa_private_key::from_protobuf(bufferview_t pkcs) { return ::from_protobuf<rsa_private_key>(pkcs); }




rsa_private_key p2p::crypto::generate_keypair(uint32_t bits) 
{
    Botan::AutoSeeded_RNG rng {};

    if (bits == 0) {
        return {
            /* n  0x30 */to_buffer(rng.random_vec(0x30)),
            /* e  0x04 */to_buffer(rng.random_vec(0x04)),
            /* d  0x20 */to_buffer(rng.random_vec(0x20)),
            /* p  0x18 */to_buffer(rng.random_vec(0x18)),
            /* q  0x18 */to_buffer(rng.random_vec(0x18)),
            /* dp 0x28 */to_buffer(rng.random_vec(0x28)),
            /* dq 0x28 */to_buffer(rng.random_vec(0x28)),
            /* qi 0x10 */to_buffer(rng.random_vec(0x10))
        };
    }

    auto privKey = Botan::RSA_PrivateKey{ rng, bits };

    return {
        /* n  0x30 */to_buffer(privKey.get_n()),
        /* e  0x04 */to_buffer(privKey.get_e()),
        /* d  0x20 */to_buffer(privKey.get_d()),
        /* p  0x18 */to_buffer(privKey.get_p()),
        /* q  0x18 */to_buffer(privKey.get_q()),
        /* dp 0x28 */to_buffer(privKey.get_d1()),
        /* dq 0x28 */to_buffer(privKey.get_d2()),
        /* qi 0x10 */to_buffer(privKey.get_c())
    };
}

