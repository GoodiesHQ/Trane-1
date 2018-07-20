#ifndef TRANE_CONNECTION_HPP
#define TRANE_CONNECTION_HPP

#include "commands.hpp"
#include "utils.hpp"

#include <functional>
#include <msgpack.hpp>

namespace trane
{
    enum ConnectionState : unsigned char
    {
        INIT,
        CONECTING,
        CONNECTED,
        FAILED,
    };


    template <size_t BufSize = TRANE_BUFSIZE>
    class Connection
    {
        static_assert(BufSize && ((BufSize & 0x3fff) == 0), "BufSize must be a non-zero multiple of 1024");
    public:
        typedef std::function<void(uint64_t)> ErrorHandler;

        Connection(asio::io_service& ios, uint64_t sessionid, ErrorHandler eh);

        /*
         * Event handlers.
         */
        virtual void handle_write(std::shared_ptr<buf_t> buf, const asio::error_code& err, size_t bytes_transferred);
        virtual void handle_read(const asio::error_code& err, size_t bytes_transferred);
        virtual void handle_error(const asio::error_code& err);

        /*
         * Command handlers.
         */
        virtual void handle_cmd_connect(const msgpack::object& obj);        // server
        virtual void handle_cmd_assign(const msgpack::object& obj);         // client
        virtual void handle_cmd_ping(const msgpack::object& obj);           // server
        virtual void handle_cmd_pong(const msgpack::object& obj);           // client
        virtual void handle_cmd_tunnel_req(const msgpack::object& obj);     // client
        virtual void handle_cmd_tunnel_res(const msgpack::object& obj);     // server

        /*
         * Command initiators
         */
        template<typename F, typename... Args> void send_cmd(F func, Args&&... args);

        void send_cmd_connect(const std::string& name);
        void send_cmd_assign(uint64_t sessionid);
        void send_cmd_ping(const std::string& message);
        void send_cmd_pong(const std::string& message);
        void send_cmd_tunnel_req(const std::string& host_server, unsigned short port_server,
                                 const std::string& host_client, unsigned short port_client,
                                 unsigned char trane_type, uint64_t tunnelid);
        void send_cmd_tunnel_res(uint64_t tunnelid, bool success, const std::string& message);

        // reserve unpacker buffer and async read.
        virtual void do_read();

        // get a const ref to the internal socket
        const tcp::socket& socket() const;

        // get state
        ConnectionState state() const;
        uint64_t sessionid() const;
        void set_sessionid(uint64_t sessionid);


    protected:
        void set_state(ConnectionState state);

        asio::io_service& m_ios;
        tcp::socket m_socket;
        ConnectionState m_state{INIT};
        ErrorHandler m_eh;
        uint64_t m_sessionid;
        msgpack::unpacker m_unpacker;
        mutable std::mutex m_mu;
    };
}


template<size_t BufSize>
trane::Connection<BufSize>::Connection(asio::io_service& ios, uint64_t sessionid, ErrorHandler eh)
    : m_ios{ios}, m_socket{ios}, m_eh{eh}, m_sessionid{sessionid}
{}


template<size_t BufSize>
void trane::Connection<BufSize>::do_read()
{
    if(state() == FAILED)
    {
        std::cerr << "Failed State. Exiting.\n";
        return;
    }
    m_unpacker.reserve_buffer(BufSize);
    m_socket.async_read_some(asio::buffer(m_unpacker.buffer(), TRANE_BUFSIZE),
        [this](const asio::error_code& err, size_t bytes_transferred){
            this->handle_read(err, bytes_transferred);
        }
    );
}


template<size_t BufSize>
void trane::Connection<BufSize>::handle_error(const asio::error_code& err)
{
    std::cerr << "ERROR: " << err.category().name() << ": " << err.message() << '\n';
    this->m_eh(m_sessionid);
}


template<size_t BufSize>
const tcp::socket& trane::Connection<BufSize>::socket() const
{
    return m_socket;
}


template<size_t BufSize>
void trane::Connection<BufSize>::handle_write(std::shared_ptr<buf_t> buf, const asio::error_code& err, size_t bytes_transferred)
{
    (void)bytes_transferred;
    if(err)
    {
        handle_error(err);
        return;
    }
    (void)buf;
}


template<size_t BufSize>
void trane::Connection<BufSize>::handle_read(const asio::error_code& err, size_t bytes_transferred)
{
    if(err)
    {
        std::cerr << "Error: " << err.message() << std::endl;
    }

    m_unpacker.buffer_consumed(bytes_transferred);
    msgpack::object_handle handle;
    while(m_unpacker.next(handle))
    {
        trane::command_t cmd;
        auto tmp = handle.get();
        tmp.convert(cmd);
        auto obj = std::get<1>(cmd);
        switch(std::get<0>(cmd))
        {
        case TraneCommand::CONNECT:
            handle_cmd_connect(obj);
            break;
        case TraneCommand::ASSIGN:
            handle_cmd_assign(obj);
            break;
        case TraneCommand::PING:
            handle_cmd_ping(obj);
            break;
        case TraneCommand::PONG:
            handle_cmd_pong(obj);
            break;
        case TraneCommand::TUNNEL_REQ:
            handle_cmd_tunnel_req(obj);
            break;
        case TraneCommand::TUNNEL_RES:
            handle_cmd_tunnel_res(obj);
            break;
        }
    }
    this->do_read();
}


template<size_t BufSize>
trane::ConnectionState trane::Connection<BufSize>::state() const
{
    SCOPELOCK(m_mu);
    return m_state;
}


template<size_t BufSize>
void trane::Connection<BufSize>::set_state(ConnectionState state)
{
    SCOPELOCK(m_mu);
    m_state = state;
}


template<size_t BufSize>
uint64_t trane::Connection<BufSize>::sessionid() const
{
    return m_sessionid;
}


template<size_t BufSize>
void trane::Connection<BufSize>::set_sessionid(uint64_t sessionid)
{
    m_sessionid = sessionid;
}

/*
 * Default handlers do nothing with the object and schedule no async events.
 */


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_connect(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_assign(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_ping(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_pong(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_tunnel_req(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
void trane::Connection<BufSize>::handle_cmd_tunnel_res(const msgpack::object& obj) { NOP(obj); }


template<size_t BufSize>
template<typename F, typename... Args>
void trane::Connection<BufSize>::send_cmd(F func, Args&&... args)
{
    auto buf = std::make_shared<buf_t>();
    try{
        func(*buf, std::forward<Args>(args)...);
    } catch(const msgpack::type_error& err)
    {
        std::cerr << "Error Sending Command: " << err.what() << std::endl;
        return;
    }

    asio::async_write(m_socket, asio::buffer(buf->data(), buf->size()),
        [this, buf](const asio::error_code& err, size_t bytes_transferred){
            this->handle_write(buf, err, bytes_transferred);
        }
    );
}


template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_connect(const std::string& name) {
    this->send_cmd(cmd_connect, name);
}

template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_assign(uint64_t sessionid) {
    this->send_cmd(cmd_assign, sessionid);
}

template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_ping(const std::string& message) {
    this->send_cmd(cmd_ping, message);
}

template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_pong(const std::string& message) {
    this->send_cmd(cmd_pong, message);
}

template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_tunnel_req(const std::string& host_server, unsigned short port_server,
                                                     const std::string& host_client, unsigned short port_client,
                                                     unsigned char trane_type, uint64_t tunnelid)
{
    this->send_cmd(cmd_tunnel_req, host_server, port_server, host_client, port_client, trane_type, tunnelid);
}

template<size_t BufSize>
void trane::Connection<BufSize>::send_cmd_tunnel_res(uint64_t tunnelid, bool success, const std::string& message)
{
    this->send_cmd(cmd_tunnel_res, tunnelid, success, message);
}

#endif
