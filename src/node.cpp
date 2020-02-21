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
#include <deque>

// https://github.com/libp2p/js-libp2p/blob/master/examples/echo/src/libp2p-bundle.js
// https://github.com/libp2p/js-libp2p/blob/master/src/index.js

using namespace std::placeholders;

class ASIO_Singleton
{
public:
    ASIO_Singleton() : _work(io_service), _loop([this]() { 
        io_service.run();
    })
    { }

    ~ASIO_Singleton() {
        io_service.stop(); 
        _loop.join(); 
    }

    asio::io_service io_service;

private:
    asio::io_service::work _work;
    std::thread _loop;
};

static ASIO_Singleton ASIO;


namespace {
    const struct node_error_category : std::error_category
    {
        const char* name() const noexcept override { return "p2p::node"; }

        std::string message(int ev) const override
        {
            switch (static_cast<node_error>(ev))
            {
            case node_error::no_ipfs_address:
                return "the provided multiaddress is not an IPFS address";

            default:
                return "(unrecognized error)";
            }
        }
    } errcat{};
}

std::error_code p2p::make_error_code(node_error e)
{
    return { static_cast<int>(e), errcat };
}


class echo_server : public std::enable_shared_from_this<echo_server>
{
public:
    echo_server()
        : _socket(ASIO.io_service), _data(1024)
    { }

    ~echo_server()
    { 
        _socket.close();
    }

    _tcp::socket& socket() { return _socket; }

    void start()
    {
        _socket.async_read_some(asio::buffer(_data), std::bind(&echo_server::handle_read, shared_from_this(), _1, _2));
    }

private:
    void handle_read(asio::error_code error, size_t bytes_transferred)
    {
        //{//DEBUG
        //    std::cout << "echo_server:handle_read:error:" << error.message() << std::endl;
        //    std::cout << "                       :bytes:" << bytes_transferred << std::endl;
        //}
        if (!error)
        {
            asio::async_write(_socket, asio::buffer(_data.data(), bytes_transferred), std::bind(&echo_server::handle_write, shared_from_this(), _1));
        }
    }

    void handle_write(asio::error_code error)
    {
        //{//DEBUG
        //    std::cout << "echo_server:handle_write:error:" << error.message() << std::endl;
        //}

        if (!error)
        {
            _socket.async_read_some(asio::buffer(_data), std::bind(&echo_server::handle_read, shared_from_this(), _1, _2));
        }
    }

    _tcp::socket _socket;
    std::vector<char> _data;
};

class echo_client : public p2p::connection, public std::enable_shared_from_this<echo_client>
{
    public:
        echo_client()
            : _socket(ASIO.io_service)
        { }

        ~echo_client()
        {
            _socket.close();
        }

        _tcp::socket& socket() { return _socket; }

        void async_connect(_tcp::resolver::iterator endpoint_iterator, const std::function<void(std::error_code)>& handler)
        {
            auto self(shared_from_this());
            asio::async_connect(_socket, endpoint_iterator, [self, handler](std::error_code error, _tcp::resolver::iterator it)
            {
                //{//DEBUG
                //    std::cout << "echo_client:async_connect:error:" << error.message() << std::endl;
                //    std::cout << "                         :it   :" << it->host_name() << ":" << it->service_name() << std::endl;
                //}
                handler(error);
            });
        }

        void close()
        {
            ASIO.io_service.post([this]() { _socket.close(); });
        }

        void write(const buffer_t& msg)
        {
            auto self(shared_from_this());
            ASIO.io_service.post([self, this, msg]()
            {
                bool write_in_progress = !write_queue.empty();
                write_queue.push_back(msg);
                if (!write_in_progress)
                {
                    do_write();
                }
            });
        }

        void read(const std::function<void(std::error_code, const buffer_t&)>& handler)
        {
            auto self(shared_from_this());
            _socket.async_read_some(asio::buffer(read_buffer, max_length), [self, this, handler](std::error_code error, std::size_t length)
            {
                //{//DEBUG
                //    std::cout << "echo_client:async_read:error :" << error.message() << std::endl;
                //    std::cout << "                      :length:" << length << std::endl;
                //}

                if (error) {
                    _socket.close();
                    return handler(error, {});
                }

                handler({}, buffer_t{ &read_buffer[0], &read_buffer[length] });
            });
        }

    private:
        void do_write()
        {
            auto self(shared_from_this());
            asio::async_write(_socket, asio::buffer(write_queue.front().data(), write_queue.front().size()), [self, this](std::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    write_queue.pop_front();
                    if (!write_queue.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    _socket.close();
                }
            });
        }

    private:
        _tcp::socket _socket;
        enum { max_length = 1024 };
        char read_buffer[max_length];
        std::deque<buffer_t> write_queue;
};


class p2p::node::nodeimpl
{

public:
    nodeimpl()
        : _acceptor(ASIO.io_service), _resolver(ASIO.io_service)
    {
        local_endpoints();
    }

    ~nodeimpl()
    {
        stop();
    }

    std::vector<addr_buffer<>> local_endpoints()
    {
        // loopback   : /ip4/127.0.0.1 
        //            : /ip6/::1
        // LAN        : /ip4/172.16.0.0/12
        //            : /ip4/192.168.0.0/16
        // link-local : /ip4/169.254.0.0/16 (4 addresses with go-ipfs)
        //

        static auto memo = std::vector<addr_buffer<>>{};
        if (memo.empty())
        {
            // loopback
            memo.push_back({ ip4, asio::ip::address_v4::loopback().to_string() });
            memo.push_back({ ip6, asio::ip::address_v6::loopback().to_string() });

            // private addresses
            auto it = _resolver.resolve({ asio::ip::host_name(), "" });
            while (it != _tcp::resolver::iterator{})
            {
                auto addr = (it++)->endpoint().address();
                std::cout << (addr.is_v6() ? "ipv6 address: " : "ipv4 address: ") << addr.to_string() << std::endl;

                memo.push_back({ addr.is_v6() ? ip6 : ip4, addr.to_string() });
            }

            // link-local
        }
        return memo;
    }


    multiaddr listen(const multiaddr& ma)
    {
        auto protocol = ma[0].addr() == ip4 ? _tcp::v4() : ma[0].addr() == ip6 ? _tcp::v6() : throw std::invalid_argument("must be IPv4 or IPv6 multiaddr");
        auto host = ma[0].str();
        auto port = std::stoi(ma[1].str());

        _acceptor.open(protocol);
        _acceptor.set_option(_tcp::acceptor::reuse_address(true));
        _acceptor.bind(_tcp::endpoint(asio::ip::address::from_string(host), port));
        _acceptor.listen();

        accept_new_connection();

        auto lep = _acceptor.local_endpoint();
        auto addr = lep.address();

        return { (ma[0].addr() == ip4 ? "/ip4/" : "/ip6/") + lep.address().to_string() + "/tcp/" + std::to_string(lep.port()) };
    }

    void stop()
    {
        _acceptor.close();
        _resolver.cancel();
    }

    template <class Iterator>
    void async_connect(const Iterator& current, const Iterator& end, const DialHandler& handler)
    {
        auto ma = *current;

        auto host = ma[0].str();
        auto port = ma[1].str();

        _resolver.async_resolve({ host, port }, [this, current, end, handler](asio::error_code error, _tcp::resolver::iterator it) {
            //{//DEBUG
            //    std::cout << "on_async_resolve:error:" << error.message() << std::endl;
            //    auto copyIt = it;
            //    while (copyIt != _tcp::resolver::iterator{}) {
            //        std::cout << "                :it   :" << copyIt->host_name() << ":" << copyIt->service_name() << std::endl;
            //        copyIt++;
            //    }
            //}

            if (error) {
                auto next = current;
                next++;
                if (next == end) return handler(error, nullptr);
                return async_connect(next, end, handler);
            }

            auto conn = std::make_shared<echo_client>();
            conn->async_connect(it, [=](std::error_code error) {
                return error ? handler(error, {}) : handler({}, conn);
            });
        });
    };

private:
    void accept_new_connection()
    {
        auto new_session = std::make_shared<echo_server>();
        _acceptor.async_accept(new_session->socket(), [this, new_session](asio::error_code error)
        {
            //{//DEBUG
            //    std::cout << "on_async_accept:error :" << error.message() << std::endl;
            //    std::cout << "               :local :" << new_session->socket().local_endpoint() << std::endl;
            //    std::cout << "               :remote:" << new_session->socket().remote_endpoint() << std::endl;
            //}

            if (error) return;

            new_session->start();
            accept_new_connection();
        });
    }

private:
    _tcp::acceptor _acceptor;
    _tcp::resolver _resolver;
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

    auto newma = _impl->listen(ma);

    _info.update(ma, newma);
}

void node::close()
{
    _impl->stop();
}

void node::dial(const peerinfo& info, const DialHandler& handler)
{
    return dialProtocol(info, "", std::move(handler));
}
void node::dial(const peerid& id, const DialHandler& handler)
{ 
    return dial(_store.at(id), std::move(handler));
}
void node::dial(const multiaddr& info, const DialHandler& handler)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dial(peerid{ as_string(proto.data()) }, std::move(handler));
        }
    }
    return handler(node_error::no_ipfs_address, nullptr);
}


void node::dialProtocol(const peerinfo& info, const std::string& protocol, const DialHandler& handler)
{
    _impl->async_connect(info.addrs().begin(), info.addrs().end(), std::move(handler));
}
void node::dialProtocol(const peerid& id, const std::string& protocol, const DialHandler& handler)
{
    return dialProtocol(_store.at(id), protocol, std::move(handler));
}
void node::dialProtocol(const multiaddr& info, const std::string& protocol, const DialHandler& handler)
{
    for (auto proto : info.protocols()) {
        if (proto.addr() == multiformats::ipfs) {
            return dialProtocol(peerid{ as_string(proto.data()) }, protocol, std::move(handler));
        }
    }
    return handler(node_error::no_ipfs_address, nullptr);
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
