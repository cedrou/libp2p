#include <p2p/switch.h>

using namespace p2p;
using namespace multiformats;

// https://github.com/libp2p/libp2p-switch/blob/master/src/index.js
// https://github.com/libp2p/libp2p-switch/blob/master/src/transports.js

void switchhub::start()
{
    // Only listen on transports we actually have addresses for
    for (auto kv : _transports) {
        if (kv.second->match_any_of(_info.addrs())) { // should be always true if created by a node
            // Listen on the given transport with default handler
            listen(kv.first);
        }
    }
}

void switchhub::stop()
{

}


void switchhub::add(sp_transport transport)
{
    if (_transports.find(transport->id()) != _transports.end())
        throw std::invalid_argument("There is already a transport with this key.");

    _transports.insert({ transport->id() , transport });
}

//void switchhub::dial(const key_t& key, const peerinfo& pi)
//{
//
//}
//

void protocol_muxer();

void switchhub::listen(const transport::id_t& key, const listener::handler_t& handler)
{
    if (!handler) {

    }
}
//
//void switchhub::close(const key_t& key)
//{
//
//}
