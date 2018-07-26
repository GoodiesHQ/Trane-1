#ifndef TRANE_SESSION_HPP
#define TRANE_SESSION_HPP
#include "asio_standalone.hpp"
#include "commands.hpp"
#include "connection.hpp"
#include "container.hpp"
#include "server_proxy.hpp"

#include <random>
#include <msgpack.hpp>
#include <functional>
#include <array>
#include <iostream>

namespace trane
{
    /*
     * A session object represents a single connection from a Trane Client to a Trane Server
     */
    template <size_t BufSize = TRANE_BUFSIZE>
    class Session : public Connection<BufSize>
    {
        using ErrorHandler = typename Connection<BufSize>::ErrorHandler;

    public:
        Session(asio::io_service& ios, uint64_t sessionid, ErrorHandler error_handler);
        void start();
        void create_tunnel(const asio::ip::address& trane_server, TraneType trane_type, const std::string& client_host, uint16_t client_port);

    protected:
        /*
         * Send a request to the client to establish a new tunnel
         */
        void send_request(const ParamTunnelReq& param);

        /*
         * Generating tunnels (max_retries = the maximum number of random ports to check before returning with an error
         */
        std::shared_ptr<ServerProxy<tcp, BufSize>> gen_tcp_tunnel(uint64_t& id, int max_retries=25);
        std::shared_ptr<ServerProxy<tcp, BufSize>> gen_udp_tunnel(uint64_t& id, int max_retries=25);

        /*
         * Handle server-side commands
         */
        void handle_cmd_connect(const msgpack::object& obj);
        void handle_cmd_ping(const msgpack::object& obj);

    private:
        Container<ServerProxy<tcp, BufSize>> m_tcp_tunnels;
        // Container<ServerProxy<udp, BufSize>> m_udp_tunnels;
    };
}


/*
 * TODO: FINISH THIS
 */

template<size_t BufSize>
std::shared_ptr<trane::ServerProxy<tcp, BufSize>> trane::Session<BufSize>::gen_tcp_tunnel(uint64_t& id, int max_retries)
{
    uint16_t port1 = 0, port2 = 0;

    for(int i = 0; i < max_retries; ++i)
    {
        auto& random = m_tcp_tunnels.random();
        port1 = port1 ? port1 : random.template randrange<uint16_t>(TRANE_ADMIN_PORT_BEGIN, TRANE_ADMIN_PORT_END);
        port2 = port2 ? port2 : random.template randrange<uint16_t>(TRANE_CLIENT_PORT_BEGIN, TRANE_CLIENT_PORT_END);
        try
        {
            auto tunnel = std::make_shared<trane::ServerProxy<tcp, BufSize>>(this->m_ios, port1, port2);
            id = m_tcp_tunnels.add(tunnel);
            tunnel->set_tunnelid(id);
            tunnel->listen();
            return tunnel;
        }
        catch(asio::system_error& err)
        {
            // TODO: determine offending port so we don't re-generate both.
            LOG(WARNING) << "regenerating ports";
            port1 = 0;
            port2 = 0;
            continue;
        }
    }
    LOG(ERROR) << "could not find open ports";
    return nullptr;
}


        /*
         * void send_cmd_tunnel_req(const std::string& host_server, uint16_t port_server,
                                 const std::string& host_client, uint16_t port_client,
                                 unsigned char trane_type, uint64_t tunnelid);
         */

template<size_t BufSize>
void trane::Session<BufSize>::create_tunnel(const asio::ip::address& trane_server, TraneType trane_type, const std::string& client_host, uint16_t client_port)
{
    if(trane_type == TraneType::TCP)
    {
        uint64_t tunnelid;
        auto tunnel = this->gen_tcp_tunnel(tunnelid);
        if(tunnel == nullptr)
        {
            return;
        }
        this->send_cmd_tunnel_req(trane_server.to_string(), tunnel->port_up(), client_host, client_port, static_cast<unsigned char>(trane_type), tunnelid);
    }
}


template<size_t BufSize>
void trane::Session<BufSize>::handle_cmd_connect(const msgpack::object& obj)
{
    ParamConnect param;
    obj.convert(param);
    this->set_state(CONNECTED);

    LOG(SUCCESS) << "Site " << P0(param) << " joined with ID " << std::setfill('0') << std::setw(16) << std::hex << this->m_sessionid;
    this->send_cmd_assign(this->m_sessionid);
}


template<size_t BufSize>
void trane::Session<BufSize>::handle_cmd_ping(const msgpack::object& obj)
{
    ParamPing param;
    obj.convert(param);
    auto& ping = P0(param);

    LOG(DEBUG) << "Received PING(\"" << ping << "\")";
    this->send_cmd_pong("PONG");
}


template<size_t BufSize>
trane::Session<BufSize>::Session(asio::io_service& ios, uint64_t sessionid, ErrorHandler eh)
    : Connection<BufSize>(ios, sessionid, eh)
{
    LOG(VERBOSE);
}


template<size_t BufSize>
void trane::Session<BufSize>::start()
{
    LOG(DEBUG) << "starting session";
    this->do_read();
};


#endif

