#ifndef TRANE_UTILS_HPP
#define TRANE_UTILS_HPP

#include <mutex>
#include <chrono>

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
    using buf_t = msgpack::sbuffer;
}

#endif
