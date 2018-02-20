#pragma once

#include <memory>
#include <p2p/connection.h>
#include <multiformats/multiaddr.h>

namespace p2p {

    class listener {
    public:
        // This method creates a listener on the transport.
        static listener create(std::function<void(const connection&)> handler);

    };

    class transport {
    public:
        // This method dials a transport to the Peer listening on multiaddr.
        virtual std::unique_ptr<connection> dial(const multiformats::multiaddr&) = 0;

        // This method creates a listener on the transport.
        virtual listener create_listener(std::function<void(const connection&)> handler) = 0;

    };
}