#ifndef TRANE_CLIENT_HPP
#define TRANE_CLIENT_HPP
#include "asio_standalone.hpp"
#include "client_proxy.hpp"
#include "commands.hpp"
#include "connection.hpp"
#include "container.hpp"
#include "resolver.hpp"
#include "utils.hpp"

#include <msgpack.hpp>
#include <iostream>
#include <string>

namespace trane
{
    /*
     * Trane server wil always use TCP to communicate with clients.
     */
    template<size_t BufSize = TRANE_BUFSIZE>
    class Client : public Connection<BufSize>
    {
        using ErrorHandler = typename Connection<BufSize>::ErrorHandler;

    public:
        Client(asio::io_service& ios, const std::string& name, const std::string& host, unsigned short port, ErrorHandler eh);
        void start();

    protected:

        void handle_connect(const asio::error_code& err);

        /*
         * Command handlers
         */
        void handle_cmd_assign(const msgpack::object& obj);
        void handle_cmd_pong(const msgpack::object& obj);
        void handle_cmd_tunnel_req(const msgpack::object& obj);

        asio::steady_timer m_heartbeat_timer;   // timer for executing PING commands for heartbeats
        trane::Resolver<tcp> m_resolver;        // a DNS resolver for creating TCP endpoints
        std::string m_name, m_host;             // store the client's site name and remote host/port
        unsigned short m_port;

    private:
        Container<ClientProxy<tcp, BufSize>> m_tcp_tunnels;
        // Container<ClientProxy<udp, BufSize>> m_udp_tunnels;
    };
}


template<size_t BufSize>
void trane::Client<BufSize>::handle_cmd_assign(const msgpack::object& obj)
{
    ParamAssign param;
    obj.convert(param);

    this->set_sessionid(std::get<0>(param));
    LOG(DEBUG) << "Client received " << std::setfill('0') << std::setw(16) << std::hex << this->m_sessionid;
    this->send_cmd_ping("PING");
}


template<size_t BufSize>
void trane::Client<BufSize>::handle_cmd_pong(const msgpack::object& obj)
{
    ParamPong param;
    obj.convert(param);

    auto& pong = std::get<0>(param);

    LOG(DEBUG) << "Client received PONG(\"" << pong << "\")";

    m_heartbeat_timer.expires_after(SEC(10));
    m_heartbeat_timer.async_wait(
        [this](const asio::error_code& err)
        {
            if(err)
            {
                this->handle_error(err);
            }
            else
            {
                this->send_cmd_ping("PING");

            }
        }
    );
}


//using ParamTunnelReq = std::tuple<std::string, uint16_t, std::string, uint16_t, unsigned char, uint64_t>;
template<size_t BufSize>
void trane::Client<BufSize>::handle_cmd_tunnel_req(const msgpack::object& obj)
{
    ParamTunnelReq param;
    obj.convert(param);

    if(P4(param) == TraneType::TCP)
    {
        auto trane_server = tcp::endpoint(asio::ip::address::from_string(P0(param)), P1(param));
        auto tunnel = std::make_shared<ClientProxy<tcp, BufSize>>(this->m_ios, trane_server, P2(param), P3(param));
        uint64_t id = m_tcp_tunnels.add(tunnel);
        tunnel->set_tunnelid(id);
        tunnel->set_sessionid(this->m_sessionid);

        LOG(DEBUG) << "Up: " << P0(param) << ':' << P1(param) << " ~ Down: " << P2(param) << ':' << P3(param)
            << " with ID " << std::setfill('0') << std::setw(16) << std::hex << P5(param);

        tunnel->start();
    }
}


template<size_t BufSize>
trane::Client<BufSize>::Client(asio::io_service& ios, const std::string& name, const std::string& host, unsigned short port, ErrorHandler eh)
    : Connection<TRANE_BUFSIZE>(ios, 0, eh), m_heartbeat_timer{ios}, m_resolver{ios}, m_name{name}, m_host{host}, m_port{port}
{ }


template<size_t BufSize>
void trane::Client<BufSize>::start()
{
    m_resolver.resolve(m_host, m_port,
        [this](const asio::error_code& err, tcp::resolver::iterator endpoints)
        {
            if(err)
            {
                std::cout << "Client resolution failure: " << err.message() << '\n';
                return;
            }
            this->m_socket.async_connect(*endpoints,
                [this](const asio::error_code& err)
                {
                    this->handle_connect(err);
                }
            );
        }
    );
}


template<size_t BufSize>
void trane::Client<BufSize>::handle_connect(const asio::error_code& err)
{
    if(err)
    {
        this->handle_error(err);
        return;
    }
    //this->send_cmd_ping("PING");
    this->send_cmd_connect(this->m_name);
    this->do_read();
}

#endif
