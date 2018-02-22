#include <p2p/transports/tcp.h>

#ifdef _MSC_VER
#include <SDKDDKVer.h>
#endif

#define ASIO_STANDALONE 
#include <asio.hpp>

using namespace p2p;
using namespace multiformats;

using _tcp = asio::ip::tcp;


std::unique_ptr<connection> p2p::transports::tcp::dial(const multiaddr& ma)
{
    auto host = ma[0].str();
    auto port = ma[1].str();

    asio::io_service aios;

    _tcp::resolver resolver{ aios };
    auto endpoint = resolver.resolve(_tcp::resolver::query(ma[0].str(), ma[1].str()));

    _tcp::socket socket(aios);

    asio::connect(socket, endpoint);

    return nullptr;
}

listener p2p::transports::tcp::create_listener(std::function<void(const connection&)> handler)
{
    return listener::create(handler);
}

bool p2p::transports::tcp::match(const multiformats::multiaddr& addr) const
{
    if (addr.has(p2p_circuit)) return false;
    return is_tcp(addr.decapsulate(ipfs));
}

//std::vector<multiaddr> p2p::transports::tcp::filter(const std::initializer_list<multiformats::multiaddr>& addresses)
//{
//    auto result = std::vector<multiaddr>();
//    std::copy_if(std::begin(addresses), std::end(addresses), std::back_inserter(result), [](auto ma) {
//        if (ma.has(p2p_circuit)) return false;
//        ma = ma.decapsulate(ipfs);
//        return is_tcp(ma);
//    });
//    return result;
//}
