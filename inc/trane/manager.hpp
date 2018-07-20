#ifndef TRANE_MANAGER_HPP
#define TRANE_MANAGER_HPP

#include <memory>
#include <unordered_map>

#include "asio_standalone.hpp"
#include "proxy.hpp"
#include "server.hpp"


namespace trane
{
    template<size_t BufSize>
    class Manager
    {
        typedef uint64_t SessionID;
        typedef uint64_t TunnelID;

    public:
        Manager(asio::io_service& ios);

    protected:
        asio::io_service& m_ios;
    };
}


#endif
