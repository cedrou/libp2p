#pragma once

#include "switch.h"
#include <multiformats\multiaddr.h>
#include <system_error>

namespace p2p {


    class node {
        using modules_t = void*;
        
        using StartHandler = std::function<void(const std::error_code&, const multiformats::multiaddr&)>;
        using DialHandler = std::function<void(const std::error_code&, std::shared_ptr<connection>)>;

    public:
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


        void dial(const peerinfo& info, const DialHandler& handler);
        void dial(const peerid& info, const DialHandler& handler);
        void dial(const multiformats::multiaddr& info, const DialHandler& handler);
        
        void dialProtocol(const peerinfo& info, const std::string& protocol, const DialHandler& handler);
        void dialProtocol(const peerid& info, const std::string& protocol, const DialHandler& handler);
        void dialProtocol(const multiformats::multiaddr& info, const std::string& protocol, const DialHandler& handler);

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

    enum class node_error
    {
        no_ipfs_address = 1,
    };

    inline std::error_code make_error_code(node_error);
}

namespace std
{
    template <>
    struct is_error_code_enum<p2p::node_error> : true_type {};
}
