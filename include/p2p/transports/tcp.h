#pragma once

#include <p2p/connection.h>
#include <p2p/transport.h>

namespace p2p {
namespace transports {

    class tcp : public transport
    {
    public:
        virtual inline id_t id() const { return "TCP"; }

        // transport interface
        virtual std::unique_ptr<connection> dial(const multiformats::multiaddr& ma);
        virtual listener create_listener(std::function<void(const connection&)> handler);

        virtual bool match(const multiformats::multiaddr& addr) const;
    };


}}