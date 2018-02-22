#pragma once

#include <p2p/peer.h>
#include <p2p/connection.h>
#include <p2p/transport.h>
#include <p2p/protocol.h>
#include <multiformats\multiaddr.h>

namespace p2p {

    class switchhub {
        //using handlerfunc_t = std::function<>
    public:
        switchhub(const peerinfo& info, const peerstore& store) :
            _info(info), _store(store) {};

        // Start listening on all available transports
        void start();

        void stop();

        //void handle(protocol_t protocol, )

        // transport
        void add(sp_transport transport);
        //void dial(const key_t& key, const peerinfo& pi);
        void listen(const transport::id_t& key, const listener::handler_t& handler = {});
        //void close(const key_t& key);


    private:
        peerinfo  _info;
        peerstore _store;
        std::map<transport::id_t, sp_transport> _transports;
    };

    //using sp_switch = std::shared_ptr<switchhub>;
    //using up_switch = std::unique_ptr<switchhub>;
}
