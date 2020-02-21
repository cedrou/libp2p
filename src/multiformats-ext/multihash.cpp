#include <multiformats-ext/multihash.h>

#pragma warning( push )  
#pragma warning( disable : 4250 ) // 'class1' : inherits 'class2::member' via dominance 
//#include <botan/rsa.h>
//#include <botan/auto_rng.h>
//#include <botan/asn1_oid.h>
//#include <botan/der_enc.h>
#include <botan\sha2_32.h>

#pragma warning ( pop )


//using namespace p2p::crypto;
using namespace multiformats;


buffer_t multiformats::details::digest_sha1(bufferview_t buffer)
{
    auto hash{ Botan::HashFunction::create("SHA-1") };
    hash->update(buffer.data(), buffer.size());
    auto digest = hash->final();
    auto ptr = reinterpret_cast<byte_t*>(digest.data());
    return { ptr, ptr + digest.size() };
}

buffer_t multiformats::details::digest_sha2_256(bufferview_t buffer)
{
    auto hash{ Botan::HashFunction::create("SHA-256") };
    hash->update(buffer.data(), buffer.size());
    auto digest = hash->final();
    auto ptr = reinterpret_cast<byte_t*>(digest.data());
    return { ptr, ptr + digest.size() };

}
