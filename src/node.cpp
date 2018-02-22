#include <p2p/node.h>

using namespace p2p;
using namespace multiformats;

#include <p2p/transports/tcp.h>

#ifdef _MSC_VER
#include <SDKDDKVer.h>
#endif

#define ASIO_STANDALONE 
#include <asio.hpp>
using _tcp = asio::ip::tcp;

#include <iostream>
#include <thread>

// https://github.com/libp2p/js-libp2p/blob/master/examples/echo/src/libp2p-bundle.js
// https://github.com/libp2p/js-libp2p/blob/master/src/index.js

using namespace std::placeholders;

class ASIO_Singleton
{
public:
    ASIO_Singleton() : loop([this]() { 
        asio::io_service::work work(io_service);
        io_service.run();
    })
    { }

    ~ASIO_Singleton() {
        io_service.stop(); 
        io_service.reset();
        loop.join(); 
    }

    asio::io_service io_service;

private:
    std::thread loop;
};

static ASIO_Singleton ASIO;

class p2p::node::nodeimpl
{

    class session
    {
    public:
        session()
            : socket_(ASIO.io_service)
        {
        }

        _tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            socket_.async_read_some(asio::buffer(data_, max_length), std::bind(&session::handle_read, this, _1, _2));
        }

    private:
        void handle_read(asio::error_code error, size_t bytes_transferred)
        {
            if (!error)
            {
                asio::async_write(socket_, asio::buffer(data_, bytes_transferred), std::bind(&session::handle_write, this, _1));
            }
            else
            {
                delete this;
            }
        }

        void handle_write(asio::error_code error)
        {
            if (!error)
            {
                socket_.async_read_some(asio::buffer(data_, max_length), std::bind(&session::handle_read, this, _1, _2));
            }
            else
            {
                delete this;
            }
        }

        _tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
    };

public:
    //nodeimpl(asio::io_service& io_service)
    //    : io_service(io_service)
    //{
    //}

    multiaddr listen(const multiaddr& ma)
    {
        auto protocol = ma[0].addr() == ip4 ? _tcp::v4() : ma[0].addr() == ip6 ? _tcp::v6() : throw std::invalid_argument("must be IPv4 or IPv6 multiaddr");
        auto port = std::stoi(ma[1].str());

        acceptor = std::make_unique<_tcp::acceptor>(ASIO.io_service, _tcp::endpoint(protocol, port));

        start_accept();

        auto actualma = ma;
        
        auto lep = acceptor->local_endpoint();
        auto addr = lep.address();
        port = lep.port();


        return { (ma[0].addr() == ip4 ? "/ip4/" : "/ip6/") + addr.to_string() + "/tcp/" + std::to_string(port) };
    }

    void stop()
    {
        acceptor->close();
    }

private:
    void start_accept()
    {
        session* new_session = new session();
        acceptor->async_accept(new_session->socket(), std::bind(&nodeimpl::handle_accept, this, new_session, _1));
    }

    void handle_accept(session* new_session, asio::error_code error)
    {
        if (!error)
        {
            new_session->start();
        }
        else
        {
            delete new_session;
        }

        start_accept();
    }

private:
    std::unique_ptr<_tcp::acceptor> acceptor;
};

node node::create(const peerinfo& info, const peerstore& store)
{
    //  modules
    //      transports
    //          TCP
    //          WS
    //          (?)
    //      connection
    //          muxer
    //              multiplex
    //              spdy
    //          crypto
    //              secio
    //      discovery
    //          mDNS (?)
    //          bootstrap/Railing (?)
    //          (?)
    //      DHT
    //          KadDHT (?)
    return create({}, info, store);
}

node node::create(const modules_t& modules, const peerinfo& info, const peerstore& store)
{
    return { modules, info, store };
}

node::node(const modules_t& /*modules*/, const peerinfo& info, const peerstore& store) :
    _info(info), _store(store), _switch(info, store), _impl(new nodeimpl)
{
    _started = false;

    // attach stream multiplexers (modules:connection:muxer)
    // attach crypto channels (modules:connection:crypto)
    // attach discovery mechanisms (modules:discovery)
    // dht provided components: peerRouting, contentRouting, dht (modules:dht)

    // Mount default protocols
    //Ping.mount(_switch)
}

node::~node() = default;


void node::start()
{
/*
    //TODO move this in modules
    using TransportList = std::vector<sp_transport>;
    auto knownTransports = TransportList{ std::make_shared<transports::tcp>() };
    
    // add protocols needed by each address of peerinfo
    for (auto& t : knownTransports)
        if (t->match_any_of(_info.addrs()))
            _switch.add(t);

    _switch.start();
*/
    auto& ma = *(_info.addrs().begin());

    auto newaddr = _impl->listen(ma);

    _info.update(ma, newaddr);
}

void node::close()
{
    _impl->stop();
}

void node::dial(const peerinfo& info) 
{
}
void node::dial(const peerid& id) 
{ 
    return dial(_store.at(id)); 
}
void node::dial(const multiaddr& info)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dial(peerid{ as_string(proto.data()) });
        }
    }
}

std::unique_ptr<connection> node::dialProtocol(const peerinfo& info, const std::string& protocol)
{
    return {};
}
std::unique_ptr<connection> node::dialProtocol(const peerid& id, const std::string& protocol)
{
    return dialProtocol(_store.at(id), protocol);
}
std::unique_ptr<connection> node::dialProtocol(const multiaddr& info, const std::string& protocol)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dialProtocol(peerid{ as_string(proto.data()) }, protocol);
        }
    }
    return nullptr;
}

void node::hangup(const peerinfo& info)
{

}
void node::hangup(const peerid& id)
{
    return hangup(_store.at(id));
}
void node::hangup(const multiaddr& info)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return hangup(peerid{ as_string(proto.data()) });
        }
    }
}
