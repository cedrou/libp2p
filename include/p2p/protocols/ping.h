#pragma once

#include <p2p/switch.h>
#include <p2p/protocol.h>


namespace p2p {
namespace protocols {

    class ping
    {
    public:
        const protocol_t Protocol = "/ipfs/ping/1.0.0";

        static inline void mount(switchhub* p_switch) {
            p_switch->handle(Protocol,  )
        }

        void unmount(switchhub* p_switch);
    };


}
}