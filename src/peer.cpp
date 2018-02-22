#include <p2p/peer.h>
#include <p2p/utils/template_string.h>
#include <p2p/utils/json.h>

using namespace p2p;
using namespace multiformats;

// The ID is a SHA2-256 multihash of the public key encoded in base 58.
inline peerid::id_t generate_peer_id(const peerid::pubkey_t& pubkey)
{
    auto pb = pubkey.to_protobuf();
    auto digest = multiformats::digest_of<multiformats::sha2_256>(pb);
    auto mh = multiformats::to_multihash(digest);
    return multiformats::encode<multiformats::base58btc>(mh.data());
}

inline peerid::id_t check_peer_id(const peerid::id_t& id, const peerid::pubkey_t& pubkey) {
    auto compid = generate_peer_id(pubkey);
    if (id != compid) throw std::invalid_argument("id and public key don't match");
    return id;
}

// Construct by generating a new keypair
peerid peerid::create(uint32_t bits)
{
    return peerid{ crypto::generate_keypair(bits) };
}

// Construct with id-only 
peerid::peerid(const id_t& id)
    : _privkey()
    , _pubkey()
    , _id(id)
{ }

// Construct with a pub/priv key pair
peerid::peerid(const privkey_t& privKey)
    : _privkey(privKey)
    , _pubkey(privKey.public_key())
    , _id(generate_peer_id(_pubkey))
{ }

// Construct with a public key only
peerid::peerid(const pubkey_t& pubKey)
    : _privkey()
    , _pubkey(pubKey)
    , _id(generate_peer_id(_pubkey))
{ }

// Construct with all component and check validity
peerid::peerid(const privkey_t& privKey, const pubkey_t& pubKey, const id_t& id)
    : _privkey(privKey)
    , _pubkey(pubKey)
    , _id(id)
{
    if (privKey.public_key() != pubKey) throw std::invalid_argument("Mismatched Public and Private keys");
    if (generate_peer_id(pubKey) != id) throw std::invalid_argument("Mismatched ID and Public key");
}

// Construct by copying another peerid - check the id
peerid::peerid(const peerid& peer)
    : _privkey(peer._privkey)
    , _pubkey(peer._pubkey)
    , _id(check_peer_id(peer._id, _pubkey))
{ }


void peerid::set(const privkey_t& privKey)
{
    if (!_privkey.empty()) throw std::logic_error("Cannot change the private key of a peerid");
    if (privKey.empty()) throw std::invalid_argument("The provided private key is empty");

    auto compid = generate_peer_id(privKey.public_key());
    if (_id != compid) throw std::invalid_argument("The provided private key does not match with the peer's id");

    _privkey = privKey;
    _pubkey = privKey.public_key();
}

void peerid::set(const pubkey_t& pubKey)
{
    if (!_pubkey.empty()) throw std::logic_error("Cannot change the public key of a peerid");
    if (pubKey.empty()) throw std::invalid_argument("The provided public key is empty");

    auto compid = generate_peer_id(pubKey);
    if (_id != compid) throw std::invalid_argument("The provided public key does not match with the peer's id");

    _pubkey = pubKey;
}

std::string peerid::to_json() const
{
    return template_string{ R"({ "id":"${id}", "privKey":"${privKey}", "pubKey":"${pubKey}" })" }
        .set("${id}", _id.str())
        .set("${privKey}", encode<base64pad>(_privkey.to_protobuf()).str())
        .set("${pubKey}", encode<base64pad>(_pubkey.to_protobuf()).str())
        ;
}

peerid peerid::from_json(const std::string& json)
{
    auto id      = encoded_string<base58btc>{ json::getstring(json, "id") };
    auto privKey = encoded_string<base64pad>{ json::getstring(json, "privKey") };
    auto pubKey  = encoded_string<base64pad>{ json::getstring(json, "pubKey") };

    return peerid( 
        crypto::rsa_private_key::from_protobuf(decode(privKey)), 
        crypto::rsa_public_key::from_protobuf(decode(pubKey)),
        id
    );
}





peerinfo p2p::make_peerinfo(uint32_t bits, std::initializer_list<multiformats::multiaddr> list /*= {}*/)
{
    return { peerid::create(bits), list };
}

peerinfo::peerinfo(const peerid& id)
    : _id(id)
{ }

peerinfo::peerinfo(const peerid& id, std::initializer_list<addr_t> list)
    : _id(id), _addrs(list)
{ }


bool peerinfo::connect(addr_t addr) 
{
    if (!has(addr)) return false;

    _connected_addr = addr;
    return true;
}

void peerinfo::disconnect() 
{
    _connected_addr = addr_t{};
}

bool peerinfo::connected() const
{
    return !_connected_addr.empty();
}

void peerinfo::add(const addr_t& addr)
{
    _addrs.insert(addr);
}

void peerinfo::update(const addr_t& before, const addr_t& after)
{
    auto it = _addrs.find(before);
    if (it == _addrs.end()) return;

    _addrs.erase(it);
    _addrs.insert(after);
}

bool peerinfo::has(const addr_t& addr) const
{
    return std::find(std::begin(_addrs), std::end(_addrs), addr) != std::end(_addrs);
}

const peerinfo& peerinfo::merge(const peerinfo& peer)
{
    // merge addresses
    for (auto addr : peer.addrs())
        add(addr);

    // set active connection state
    if (peer.connected())
        connect(peer._connected_addr);

    // update pub/priv keys
    if (_id.privkey().empty() && !peer._id.privkey().empty()) {
        _id.set(peer._id.privkey());
    }

    return *this;
}




// Checks if peer is in the container

bool peerstore::has(const peerid& peer) const 
{ 
    return _store.find(peer) != _store.end(); 
}
bool peerstore::has(const peerinfo& peer) const
{ 
    return has(peer.id());
}


// Stores a peer in the container.
const peerinfo& peerstore::insert(const peerinfo& peer, insert_policy policy)
{
    auto peerid = peer.id();
    auto it = _store.find(peerid);

    if (it == _store.end()) {
        _store.insert({ peerid, peer });
        return peer;
    }

    if (policy == insert_policy::replace) {
        _store.erase(peerid);
        _store.insert({ peerid, peer });
        return peer;
    }

    if (policy == insert_policy::merge) {
        it->second.merge(peer);
    }

    return it->second;
}
