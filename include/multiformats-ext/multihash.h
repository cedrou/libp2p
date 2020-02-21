#pragma once

#include <multiformats\multibase.h>
#include <multiformats\multihash.h>

namespace multiformats {

namespace details {

    typedef buffer_t (*DigestImpl)(bufferview_t buffer);
    buffer_t digest_sha1(bufferview_t buffer);
    buffer_t digest_sha2_256(bufferview_t buffer);

    struct hashext {
        hash_t      key;
        DigestImpl  digest;
    };

    constexpr hashext _HashExtTable[] = {
        { dynamic_hash, nullptr         },
        { sha1,         digest_sha1     },
        { sha2_256,     digest_sha2_256 },
    };

    constexpr int find_hashext_by_key(hash_t code) {
        for (auto i = 0; i < _countof(_HashExtTable); i++)
            if (_HashExtTable[i].key == code) return i;
        return 0;
    }

}


    template <hash_t _Algo>
    digest_buffer<_Algo> digest_of(bufferview_t buffer)
    {
        constexpr int _Index = details::find_hashext_by_key(_Algo);
        static_assert(_Index > 0, "digest algorithm not implemented");
        return details::_HashExtTable[_Index].digest(buffer);
    };

}