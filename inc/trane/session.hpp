#ifndef TRANE_SESSION_HPP
#define TRANE_SESSION_HPP
#include "asio_standalone.hpp"
#include <functional>
#include <array>
#include <iostream>

namespace trane
{
    template <size_t BufSize = 1024>
    class Session
    {
        typedef std::function<void(uint64_t)> ErrorHandler;

    public:
        Session(asio::io_service& ios, uint64_t sessionid, ErrorHandler error_handler);

        void start();
        void handle_read(const asio::error_code& err, size_t bytes_transferred);
        void handle_write(const asio::error_code& err);
        void set_session_id(uint64_t sessionid);
        const tcp::socket& socket();

    protected:
        tcp::socket m_socket;
        uint64_t m_sessionid;
        ErrorHandler m_eh;
        std::array<char, BufSize> m_data;
    };
}

template<size_t BufSize>
trane::Session<BufSize>::Session(asio::io_service& ios, uint64_t sessionid, ErrorHandler error_handler)
    : m_socket{ios}, m_sessionid{sessionid}, m_eh{error_handler}
{ }

template<size_t BufSize>
void trane::Session<BufSize>::start()
{
    m_socket.async_read_some(
        asio::buffer(m_data, BufSize),
        std::bind(&Session<BufSize>::handle_read, this, std::placeholders::_1, std::placeholders::_2)
    );
}

template<size_t BufSize>
void trane::Session<BufSize>::handle_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cout << "Socket read error: " << err.message() << '\n';
        m_eh(m_sessionid);
    }
    else
    {
        std::cout << "Received " << bytes_transferred << " bytes.\n";
        asio::async_write(
            m_socket,
            asio::buffer(m_data, bytes_transferred),
            std::bind(&Session<BufSize>::handle_write, this, std::placeholders::_1)
        );
    }
}

template<size_t BufSize>
void trane::Session<BufSize>::handle_write(const asio::error_code& err)
{
    if(err)
    {
        std::cout << err << std::endl;
        std::cout << "Socket write error: " << err.message() << '\n';
        m_eh(m_sessionid);
        std::cout << "OK\n";
    }
    else
    {
        m_socket.async_read_some(
            asio::buffer(m_data, BufSize),
            std::bind(&Session<BufSize>::handle_read, this, std::placeholders::_1, std::placeholders::_2)
        );
    }
}

template<size_t BufSize>
void trane::Session<BufSize>::set_session_id(uint64_t sessionid)
{
    m_sessionid = sessionid;
}

template<size_t BufSize>
const tcp::socket& trane::Session<BufSize>::socket()
{
    return m_socket;
}

#endif

