#pragma once

#include <set>
#include <multiformats-ext/multihash.h>
#include <multiformats/multiaddr.h>
#include "crypto.h"

namespace p2p {

    //
    // A peerid uniquely identify a node/peer in the p2p network
    //
    class peerid {
    public:
        using id_t      = multiformats::encoded_string<multiformats::base58btc>;
        using pubkey_t  = crypto::rsa_public_key;
        using privkey_t = crypto::rsa_private_key;

    public:
        // Creates a new peerid instance and generates a keypair for it.
        static peerid create(uint32_t bits = crypto::default_privkey_bitsize);

        peerid(const privkey_t& privKey);
        peerid(const pubkey_t& pubKey);
        peerid(const id_t& id);
        peerid(const privkey_t& privKey, const pubkey_t& pubKey, const id_t& id);
        peerid(const peerid& peer);

        void set(const privkey_t& privKey);
        void set(const pubkey_t& pubKey);

        std::string   to_json()  const;
        static peerid from_json(const std::string& json);

        inline auto sid()     const { return _id; }
        inline auto pubkey()  const { return _pubkey; }
        inline auto privkey() const { return _privkey; }

    private:
        privkey_t   _privkey;
        pubkey_t    _pubkey;
        id_t        _id;
    };


    // Comparison operators
    inline bool operator==(const peerid& _Left, const peerid& _Right) {
        return (_Left.sid() == _Right.sid());
    }
    inline bool operator!=(const peerid& _Left, const peerid& _Right) {
        return !(_Left == _Right);
    }

    inline bool operator<(const peerid& a, const peerid& b) {
        return a.sid() < b.sid();
    }


    //
    // peerinfo represents a peer on the network
    //
    class peerinfo {
        using addr_t       = multiformats::multiaddr;
        using addrs_t      = std::set<addr_t>;

    public:
        // Creates a new PeerInfo instance from an existing PeerId.
        peerinfo(const peerid& id);
        peerinfo(const peerid& id, std::initializer_list<addr_t> list);


        // Add a new address that peer can be reached at
        void add(const addr_t& addr);

        // Test if the provided address is an addrees that peer can be reached at
        bool has(const addr_t& addr) const;

        bool connect(addr_t addr);
        void disconnect();
        bool connected() const;

        const peerinfo& merge(const peerinfo& peer);

        inline const peerid&   id()             const { return _id; }
        inline const addrs_t&  addrs()          const { return _addrs; }
        inline const addr_t&   connected_addr() const { return _connected_addr; }

    private:
        peerid   _id;
        addrs_t  _addrs;
        addr_t   _connected_addr;
    };

    // Creates a new peerinfo instance and generates a new peerid for it.
    peerinfo make_peerinfo(uint32_t bits = crypto::default_privkey_bitsize, std::initializer_list<multiformats::multiaddr> list = {});
    inline peerinfo make_peerinfo(std::initializer_list<multiformats::multiaddr> list) { return make_peerinfo(crypto::default_privkey_bitsize, list); }

    inline bool operator==(const peerinfo& _Left, const peerinfo& _Right) {
        return (_Left.id() == _Right.id())
            && (_Left.addrs() == _Right.addrs())
            && (_Left.connected_addr() == _Right.connected_addr());
    }


    //
    //
    //
    class peerstore {
        using store_t = std::map<peerid, peerinfo>;

    public:
        // Construct an empty container
        peerstore() 
        {}

        // Construct a peerstore and adds the provided peerinfo
        peerstore(std::initializer_list<peerinfo> list)
        {
            for (auto info : list)
                insert(info);
        }
         


        // Checks if peer is in the container
        bool has(const peerid& peer) const;
        bool has(const peerinfo& peer) const;

        // insert a peerInfo
        enum insert_policy
        {
            nothing,
            replace,
            merge
        };
        const peerinfo& insert(const peerinfo& peer, insert_policy policy = merge);

        // get the info of the given peer
        const peerinfo& at(const peerid& id) const { return _store.at(id); }

        // remove a peer
        inline void remove(const peerid& id) { _store.erase(id); }

        inline size_t size() const { return _store.size(); }

    private:
        store_t _store;
    };
}

