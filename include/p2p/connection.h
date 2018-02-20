#pragma once

#include <multiformats\multiaddr.h>
#include <p2p\peer.h>

// https://github.com/libp2p/interface-connection

namespace p2p {

    class connection {
    public:
        // This method retrieves the observed addresses we get from the underlying transport, if any
        virtual std::vector<multiformats::multiaddr> observed_addrs() const = 0;

        // This method retrieves the a Peer Info object that contains information about the peer that this conn connects to
        virtual peerinfo get_peerinfo() const = 0;

        // This method stores a reference to the peerInfo Object that contains information about the peer that this conn connects to.
        virtual void set_peerinfo(const peerinfo& info) = 0;
    };
}