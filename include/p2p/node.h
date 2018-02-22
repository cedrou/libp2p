#pragma once

#include "switch.h"
#include <multiformats\multiaddr.h>

namespace p2p {

    class node {
        using modules_t = void*;

    public:
        ~node();
        node(node&& n) = default;

        static node create(const peerinfo& info, const peerstore& store = peerstore{});
        static node create(const modules_t& modules, const peerinfo& info, const peerstore& store);

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
        node(const modules_t& modules, const peerinfo& info, const peerstore& store);


    private:
        peerinfo   _info;
        peerstore  _store;
        switchhub  _switch;
        bool       _started;

        class nodeimpl;
        std::unique_ptr<nodeimpl> _impl;
    };
}
