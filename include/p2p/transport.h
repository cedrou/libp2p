#pragma once

#include <memory>
#include <p2p/connection.h>
#include <multiformats/multiaddr.h>

namespace p2p {

    class listener {
    public:
        using handler_t = std::function<void(const connection&)>;

        // This method creates a listener on the transport.
        static listener create(handler_t handler)
        {
            return {};
        }

    };

    class transport {
    public:
        using id_t = std::string;

    public:
        virtual id_t id() const = 0;

        // This method dials a transport to the Peer listening on multiaddr.
        virtual std::unique_ptr<connection> dial(const multiformats::multiaddr&) = 0;

        // This method creates a listener on the transport.
        virtual listener create_listener(std::function<void(const connection&)> handler) = 0;


        virtual bool match(const multiformats::multiaddr& addr) const = 0;


    public:
        template <class MultiaddrContainer>
        std::vector<multiformats::multiaddr> filter(const MultiaddrContainer& addresses)
        {
            auto result = std::vector<multiformats::multiaddr>{};

            std::copy_if(std::begin(addresses), std::end(addresses), std::back_inserter(result), [this](auto& ma) { return match(ma); });
            
            return result;
        }

        template <class MultiaddrContainer>
        bool match_any_of(const MultiaddrContainer& addresses)
        {
            return std::any_of(std::begin(addresses), std::end(addresses), [this](auto& ma) { return match(ma); });
        }
    };

    using sp_transport = std::shared_ptr<transport>;
}