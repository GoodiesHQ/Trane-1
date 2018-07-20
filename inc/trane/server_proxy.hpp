#ifndef ASIO_SERVER_PROXY_HPP
#define ASIO_SERVER_PROXY_HPP

#include "proxy.hpp"
#include <functional>


namespace trane
{
    /*
     * This proxy only receives connections.
     *
     * The ClientProxy will connect to this class and so will the admin. The admin traffic will be proxied to
     * ClientProxy which will send it to the client.
     */
    template<typename Proto, size_t BufSize>
    class ServerProxy : public Proxy<Proto, BufSize>
    {
    public:
        // All we need are two ports. One for the admin (dn) and the ClientProxy (up)
        ServerProxy(asio::io_service& ios, unsigned short port_dn, unsigned short port_up);
        virtual void listen();

        unsigned short port_up() const;
        unsigned short port_dn() const;

    protected:
        virtual void handle_up_accept(const asio::error_code& err);
        virtual void handle_dn_accept(const asio::error_code& err);

        uint64_t m_tunnelid;
        unsigned short m_port_dn, m_port_up;
        tcp::acceptor m_acc_up;
        typename Proto::acceptor m_acc_dn;
        asio::ip::address m_host_dn, m_host_up;
    };
}


/*
 * IMPLEMENTATION
 */


template<typename Proto, size_t BufSize>
trane::ServerProxy<Proto, BufSize>::ServerProxy(asio::io_service& ios, unsigned short port_dn, unsigned short port_up)
    : trane::Proxy<Proto, BufSize>::Proxy(ios), m_port_dn{port_dn}, m_port_up{port_up}, m_acc_up{ios}, m_acc_dn{ios}
{}


template<typename Proto, size_t BufSize>
void trane::ServerProxy<Proto, BufSize>::listen()
{
    std::cout << "Awaiting Trane Tunnel Connection on Port" << m_port_up << std::endl;
    m_acc_up.async_accept(this->m_sock_up,
        [this](const asio::error_code& err)
        {
            this->handle_up_accept(err);
        }
    );
}


template<typename Proto, size_t BufSize>
unsigned short trane::ServerProxy<Proto, BufSize>::port_up() const
{
    return m_port_up;
}


template<typename Proto, size_t BufSize>
unsigned short trane::ServerProxy<Proto, BufSize>::port_dn() const
{
    return m_port_dn;
}


template<typename Proto, size_t BufSize>
void trane::ServerProxy<Proto, BufSize>::handle_up_accept(const asio::error_code& err)
{
    if(err)
    {
        std::cout << "Upstream Accept Failure\n";
        return;
    }
    if(std::is_same<Proto, tcp>::value)
    {
        std::cout << "Waiting for connection on port " << std::dec << m_port_dn << "...\n";
        m_acc_dn.async_accept(this->m_sock_dn,
            [this](const asio::error_code& err)
            {
                this->handle_dn_accept(err);
            }
        );
    }
    /*
     * TODO: Implement UDP
     */
}


template<typename Proto, size_t BufSize>
void trane::ServerProxy<Proto, BufSize>::handle_dn_accept(const asio::error_code& err)
{
    if(err)
    {
        std::cout << "Downstream Accept Failure\n";
        return;
    }
    this->do_up_read();
    this->do_dn_read();
}

#endif
