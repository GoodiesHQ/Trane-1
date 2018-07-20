#ifndef ASIO_CLIENT_PROXY_HPP
#define ASIO_CLIENT_PROXY_HPP

#include "resolver.hpp"
#include "proxy.hpp"

namespace trane
{
    template<typename Proto, size_t BufSize>
    class ClientProxy : public Proxy<Proto, BufSize>
    {
    public:
        ClientProxy(asio::io_service& ios, const tcp::endpoint& trane_server, const std::string& host, unsigned short port);

        // override the default... need to ensure the client is connected on first read
        void handle_up_read(const asio::error_code& err, size_t bytes_transferred);

        void start();

        void do_up_connect();
        void do_dn_connect(size_t bytes);

        void handle_up_connect(const asio::error_code& err);
        void handle_dn_connect(size_t bytes, const asio::error_code& err);

    private:
        bool m_connected_up{false}, m_connected_dn{false};
        tcp::endpoint m_trane_server;
        std::string m_host;
        unsigned short m_port;
        trane::Resolver<Proto> m_resolver;
    };

}

/*
 * Implementation
 */
template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::start()
{
    this->do_up_connect();
}


template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::do_up_connect()
{
    this->m_sock_up.async_connect(m_trane_server,
        [this](const asio::error_code& err){
            this->handle_up_connect(err);
        }
    );
}


template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::do_dn_connect(size_t bytes_transferred)
{
    if(std::is_same<tcp, Proto>::value)
    {
        m_resolver.resolve(m_host, m_port,
            [this, bytes_transferred](const asio::error_code& err, typename Proto::resolver::iterator endpoints)
            {
                if(err)
                {
                    std::cerr << "ClientProxy resolution error\n";
                    return;
                }
                this->m_sock_dn.async_connect(*endpoints,
                    [this, bytes_transferred](const asio::error_code& err)
                    {
                        this->handle_dn_connect(bytes_transferred, err);
                    }
                );
            }
        );
    }
}


template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::handle_up_connect(const asio::error_code& err)
{
    if(err)
    {
        std::cerr << "ClientProxy Connect Error\n";
        return;
    }
    this->m_connected_up = true;
}


template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::handle_dn_connect(size_t bytes_transferred, const asio::error_code& err)
{
    if(err)
    {
        std::cerr << "ClientProxy Connect Error\n";
        return;
    }
    this->m_connected_dn = true;
    this->do_dn_write(bytes_transferred);
}


template<typename Proto, size_t BufSize>
void trane::ClientProxy<Proto, BufSize>::handle_up_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "ClientProxy Upstream Read Error: " << err.message() << std::endl;
        return;
    }
    if(!m_connected_dn)
    {
        this->do_dn_connect(bytes_transferred);
    }
    else
    {
        this->Proxy<Proto, BufSize>::handle_up_read(err, bytes_transferred);
    }
}


template<typename Proto, size_t BufSize>
trane::ClientProxy<Proto, BufSize>::ClientProxy(asio::io_service& ios, const tcp::endpoint& trane_server, const std::string& host, unsigned short port)
    : Proxy<Proto, BufSize>::Proxy(ios), m_trane_server{trane_server}, m_host{host}, m_port{port}, m_resolver{ios}
{ }

#endif
