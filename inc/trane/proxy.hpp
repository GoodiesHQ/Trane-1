#ifndef TRANE_PROXY_HPP
#define TRANE_PROXY_HPP

#include <iostream>
#include <array>
#include "asio_standalone.hpp"
#include "utils.hpp"

namespace trane
{
    template<typename Proto = tcp, size_t BufSize = TRANE_BUFSIZE>
    class Proxy
    {
        static_assert(std::is_same<Proto, tcp>::value || std::is_same<Proto, udp>::value, "Only TCP and UDP are supported");

    public:
        Proxy(asio::io_service& ios);

        /*
         * Perform socket reading
         */
        virtual void do_up_read();
        virtual void do_dn_read();

        /*
         * Handle reading of data
         */
        virtual void handle_up_read(const asio::error_code& err, size_t bytes_transferred);
        virtual void handle_dn_read(const asio::error_code& err, size_t bytes_transferred);

        /*
         * Handle writing of data
         */
        virtual void handle_up_write(const asio::error_code& err, size_t bytes_transferred);
        virtual void handle_dn_write(const asio::error_code& err, size_t bytes_transferred);

    private:
        asio::io_service& m_ios;
        tcp::socket m_sock_up;
        typename Proto::socket m_sock_dn;
        std:: array<unsigned char, BufSize> m_buf_up, m_buf_dn;
    };
}



template<typename Proto, size_t BufSize>
trane::Proxy<Proto, BufSize>::Proxy(asio::io_service& ios)
    : m_ios{ios}, m_sock_up(ios), m_sock_dn{ios}
{}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_up_read()
{
    m_sock_up.async_read_some(asio::buffer(m_buf_up.data(), BufSize),
        [this](const asio::error_code& err, size_t bytes_transferred){
            this->handle_up_read(err, bytes_transferred);
        }
    );
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_dn_read()
{
    if(std::is_same<Proto, tcp>::value)
    {
        m_sock_up.async_read_some(asio::buffer(m_buf_up.data(), BufSize),
            [this](const asio::error_code& err, size_t bytes_transferred){
                this->handle_up_read(err, bytes_transferred);
            }
        );
    }
    /*
     * TODO: Implement UDP
     */
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_up_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "Upstream Read Error: " << err.message() << std::endl;
        return;
    }

    if(std::is_same<Proto, tcp>::value)
    {
        asio::async_write(m_sock_dn, asio::buffer(m_buf_up.data(), bytes_transferred),
            [this](const asio::error_code& err, size_t bytes_transferred){
                this->handle_dn_write(err, bytes_transferred);
            }
        );
    }
    /*
     * TODO: Handle UDP
     */
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_dn_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "Upstream Read Error: " << err.message() << std::endl;
        return;
    }

    asio::async_write(m_sock_up, asio::buffer(m_buf_dn.data(), bytes_transferred),
        [this](const asio::error_code& err, size_t bytes_transferred)
        {
            this->handle_up_write(err, bytes_transferred);
        }
    );
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_up_write(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "Upstream Write Error: " << err.message() << std::endl;
        return;
    }
    NOP(bytes_transferred);
    this->do_dn_read();
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_dn_write(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "Downstream Write Error: " << err.message() << std::endl;
        return;
    }
    NOP(bytes_transferred);
    this->do_up_read();
}


/*
        virtual void handle_write(std::shared_ptr<buf_t> buf, const asio::error_code& err, size_t bytes_transferred);
        virtual void handle_read(const asio::error_code& err, size_t bytes_transferred);
        */

#endif
