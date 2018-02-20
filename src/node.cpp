#include <p2p/node.h>

using namespace p2p;
using namespace multiformats;

// https://github.com/libp2p/js-libp2p/blob/master/examples/echo/src/libp2p-bundle.js
// https://github.com/libp2p/js-libp2p/blob/master/src/index.js


node node::create(const peerinfo& info, const peerstore& store)
{
    //  modules
    //      transports
    //          TCP
    //          WS
    //          (?)
    //      connection
    //          muxer
    //              multiplex
    //              spdy
    //          crypto
    //              secio
    //      discovery
    //          mDNS (?)
    //          bootstrap/Railing (?)
    //          (?)
    //      DHT
    //          KadDHT (?)
    return create(nullptr, info, store);
}

node node::create(void* modules, const peerinfo& info, const peerstore& store)
{
    return {info};
}

void node::start()
{
    // find all protocols needed by each address of peerinfo
}

void node::close()
{

}

void node::dial(const peerinfo& info) 
{
}
void node::dial(const peerid& id) 
{ 
    return dial(_store.at(id)); 
}
void node::dial(const multiaddr& info)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dial(peerid{ as_string(proto.data()) });
        }
    }
}

std::unique_ptr<connection> node::dialProtocol(const peerinfo& info, const std::string& protocol)
{
    return {};
}
std::unique_ptr<connection> node::dialProtocol(const peerid& id, const std::string& protocol)
{
    return dialProtocol(_store.at(id), protocol);
}
std::unique_ptr<connection> node::dialProtocol(const multiaddr& info, const std::string& protocol)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dialProtocol(peerid{ as_string(proto.data()) }, protocol);
        }
    }
    return nullptr;
}

void node::hangup(const peerinfo& info)
{

}
void node::hangup(const peerid& id)
{
    return hangup(_store.at(id));
}
void node::hangup(const multiaddr& info)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return hangup(peerid{ as_string(proto.data()) });
        }
    }
}
