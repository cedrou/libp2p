#pragma once

#include <p2p/connection.h>
#include <p2p/transport.h>

namespace p2p {
namespace transports {

    class tcp : public transport, public connection
    {
    public:
        // transport interface
        virtual std::unique_ptr<connection> dial(const multiformats::multiaddr& ma);
        virtual listener create_listener(std::function<void(const connection&)> handler);
        virtual std::vector<multiformats::multiaddr> filter(const std::vector<multiformats::multiaddr>& addrs);

    };


}}