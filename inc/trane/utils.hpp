#ifndef TRANE_UTILS_HPP
#define TRANE_UTILS_HPP

#include <mutex>
#include <chrono>

#include "logging.hpp"
#include "msgpack.hpp"

#define SCOPELOCK(mu) std::lock_guard<std::mutex> __lock(mu)
#define TRANE_BUFSIZE 32 * 1024
#define NOP(x) (void)(x);

#define SEC(x)  std::chrono::seconds(x)
#define MSEC(x) std::chrono::milliseconds(x)
#define USEC(x) std::chrono::microseconds(x)


#define P0(x) std::get<0>(x)
#define P1(x) std::get<1>(x)
#define P2(x) std::get<2>(x)
#define P3(x) std::get<3>(x)
#define P4(x) std::get<4>(x)
#define P5(x) std::get<5>(x)

namespace trane {
    const unsigned TRANE_ADMIN_PORT_BEGIN = 40000;
    const unsigned TRANE_ADMIN_PORT_END = 49999;

    const unsigned TRANE_CLIENT_PORT_BEGIN = 50000;
    const unsigned TRANE_CLIENT_PORT_END = 59999;

    static_assert(TRANE_ADMIN_PORT_END - TRANE_ADMIN_PORT_BEGIN == TRANE_CLIENT_PORT_END - TRANE_CLIENT_PORT_BEGIN, "Admin and Client Ports Must Support the Same Number of Connections");

    using buf_t = msgpack::sbuffer;

    /*
     * Define packet types
     */
    enum TraneCommand : unsigned char {
        CONNECT,        // Client has initially connected. Provide the site name and other info.
        ASSIGN,         // Server acknowledges the client and provides it with an ID to use on subsequent messages
        PING,           // Heartbeat request sent by client
        PONG,           // Heartbeat response sent by Server
        TUNNEL_REQ,     // Create a Trane Tunnel request
        TUNNEL_RES,     // Tunnel creation response
    };

    enum TraneType : unsigned char {
        TCP,            // a single connection mapped to a single trane tunnel
        UDP,            // any request sent from a machine will reply to that same machine.
    };

}

#endif
