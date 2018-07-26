#ifndef TRANE_MANAGER_HPP
#define TRANE_MANAGER_HPP

#include <memory>
#include <unordered_map>

#ifdef NO_TLS
#include <websocketpp/config/asio_no_tls.hpp>
#else
#include <websocketpp/config/asio.hpp>
#endif
#include <websocketpp/server.hpp>

#include "asio_standalone.hpp"
#include "proxy.hpp"
#include "server.hpp"


namespace trane
{
    class Manager
    {

    public:
        Manager();

    protected:
        asio::io_service& m_ios;
    };
}


#endif
