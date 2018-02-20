#pragma once

#include "peer.h"
#include "connection.h"
#include <multiformats\multiaddr.h>

namespace p2p {

    class node {
    public:
        static node create(const peerinfo& info, const peerstore& store = peerstore{});
        static node create(void* modules, const peerinfo& info, const peerstore& store);

        //
        // Start the libp2p node by creating listeners on the multiaddrs the Peer wants to listen
        //
        void start();

        //
        // Stop the libp2p node by closing its listeners and open connections
        //
        void close();


        void dial(const peerinfo& info);
        void dial(const peerid& info);
        void dial(const multiformats::multiaddr& info);

        std::unique_ptr<connection> dialProtocol(const peerinfo& info, const std::string& protocol);
        std::unique_ptr<connection> dialProtocol(const peerid& info, const std::string& protocol);
        std::unique_ptr<connection> dialProtocol(const multiformats::multiaddr& info, const std::string& protocol);

        void hangup(const peerinfo& info);
        void hangup(const peerid& info);
        void hangup(const multiformats::multiaddr& info);

        const bool  started() const { return false; }

        const auto& info()    const { return _info; }
        const auto& store()   const { return _store; }

    private:
        node(const peerinfo& info) : _info(info) {};


    private:
        peerinfo  _info;
        peerstore _store;
        std::vector<
    };
}
