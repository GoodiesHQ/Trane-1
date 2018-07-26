#ifndef TRANE_PROXY_HPP
#define TRANE_PROXY_HPP

#include <iostream>
#include <array>
#include "asio_standalone.hpp"
#include "utils.hpp"
#include "logging.hpp"

namespace trane
{
    template<typename Proto = tcp, size_t BufSize = TRANE_BUFSIZE>
    class Proxy
    {
        static_assert(std::is_same<Proto, tcp>::value || std::is_same<Proto, udp>::value, "Only TCP and UDP are supported");

    public:
        Proxy(asio::io_service& ios);
        ~Proxy();

        /*
         * setters and getters for tunnel ID and session ID
         */
        void set_tunnelid(uint64_t tunnelid);
        void set_sessionid(uint64_t tunnelid);

        uint64_t tunnelid() const;
        uint64_t sessionid() const;

        /*
         * Perform socket reading
         */
        virtual void do_up_read();
        virtual void do_dn_read();


        /*
         * Perform socket writing
         */
        virtual void do_up_write(size_t bytes_transferred);
        virtual void do_dn_write(size_t bytes_transferred);

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

    protected:
        uint64_t m_tunnelid, m_sessionid;
        asio::io_service& m_ios;
        tcp::socket m_sock_up;
        typename Proto::socket m_sock_dn;
        std:: array<unsigned char, BufSize> m_buf_up, m_buf_dn;
    };
}



template<typename Proto, size_t BufSize>
trane::Proxy<Proto, BufSize>::Proxy(asio::io_service& ios)
    : m_ios{ios}, m_sock_up(ios), m_sock_dn{ios}
{
    LOG(VERBOSE);
}


template<typename Proto, size_t BufSize>
trane::Proxy<Proto, BufSize>::~Proxy()
{
    LOG(VERBOSE) << "DESTROYED";
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::set_tunnelid(uint64_t tunnelid)
{
    this->m_tunnelid = tunnelid;
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::set_sessionid(uint64_t sessionid)
{
    this->m_sessionid = sessionid;
}


template<typename Proto, size_t BufSize>
uint64_t trane::Proxy<Proto, BufSize>::sessionid() const
{
    return m_sessionid;
}


template<typename Proto, size_t BufSize>
uint64_t trane::Proxy<Proto, BufSize>::tunnelid() const
{
    return m_tunnelid;
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_up_read()
{
    LOG(VERBOSE) << "reading upstream";
    m_sock_up.async_read_some(asio::buffer(m_buf_up.data(), BufSize),
        [this](const asio::error_code& err, size_t bytes_transferred){
            this->handle_up_read(err, bytes_transferred);
        }
    );
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_dn_read()
{
    LOG(VERBOSE) << "reading downstream";
    if(std::is_same<Proto, tcp>::value)
    {
        m_sock_dn.async_read_some(asio::buffer(m_buf_dn.data(), BufSize),
            [this](const asio::error_code& err, size_t bytes_transferred){
                this->handle_dn_read(err, bytes_transferred);
            }
        );
    }
    /*
     * TODO: Implement UDP
     */
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_up_write(size_t bytes_transferred)
{
    LOG(VERBOSE) << "writing upstream";
    asio::async_write(m_sock_up, asio::buffer(m_buf_dn.data(), bytes_transferred),
        [this](const asio::error_code& err, size_t bytes_transferred)
        {
            this->handle_up_write(err, bytes_transferred);
        }
    );
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::do_dn_write(size_t bytes_transferred)
{
    LOG(VERBOSE) << "writing downstream";
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
void trane::Proxy<Proto, BufSize>::handle_up_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        LOG(ERROR) << err.message();
        return;
    }
    LOG(VERBOSE) << "received " << std::dec << bytes_transferred << " from upstream";
    this->do_dn_write(bytes_transferred);
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_dn_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        LOG(ERROR) << err.message();
        return;
    }
    LOG(VERBOSE) << "received " << std::dec << bytes_transferred << " from downstream";
    this->do_up_write(bytes_transferred);
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_up_write(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        LOG(ERROR) << err.message();
        return;
    }
    LOG(VERBOSE) << "sent " << std::dec << bytes_transferred << " bytes upstream";
    NOP(bytes_transferred);
    this->do_dn_read();
}


template<typename Proto, size_t BufSize>
void trane::Proxy<Proto, BufSize>::handle_dn_write(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        LOG(ERROR) << err.message();
        return;
    }
    LOG(VERBOSE) << "sent " << std::dec << bytes_transferred << " bytes downstream";
    NOP(bytes_transferred);
    this->do_up_read();
}

#endif
