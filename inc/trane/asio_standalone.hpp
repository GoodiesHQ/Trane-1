#ifndef TRANE_ASIO_STANDALONE
#define TRANE_ASIO_STANDALONE

#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

#include <asio.hpp>

using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

#endif
