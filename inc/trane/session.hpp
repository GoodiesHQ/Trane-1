#ifndef TRANE_SESSION_HPP
#define TRANE_SESSION_HPP
#include "asio_standalone.hpp"
#include "commands.hpp"
#include "connection.hpp"
#include "utils.hpp"

#include <msgpack.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <functional>
#include <array>
#include <iostream>

#define NOP(x) (void)(x);

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
        void after_cmd();
        void add_request(const ParamTunnelReq& param);

    protected:
        void handle_cmd_connect(const msgpack::object& obj);
        void handle_cmd_ping(const msgpack::object& obj);

    private:
        std::deque<ParamTunnelReq> m_requests;
        std::mutex m_requests_mu;
    };
}

template<size_t BufSize>
void trane::Session<BufSize>::after_cmd()
{
    {
        SCOPELOCK(m_requests_mu);
        while(!m_requests.empty())
        {
            const auto& param = m_requests.front();
            this->send_cmd_tunnel_req(P0(param), P1(param), P2(param), P3(param), P4(param));
            m_requests.pop_front();
        }
    }
}


template<size_t BufSize>
void trane::Session<BufSize>::add_request(const ParamTunnelReq& param)
{
    SCOPELOCK(m_requests_mu);
    m_requests.push_back(param);
}


template<size_t BufSize>
void trane::Session<BufSize>::handle_cmd_connect(const msgpack::object& obj)
{
    ParamConnect param;
    obj.convert(param);

    this->set_state(CONNECTED);
    std::cout << "Server Accepts Site '" << P0(param) << "'. Assigning ID " << std::setfill('0') << std::setw(16) << std::hex << this->m_sessionid << '\n';
    this->send_cmd_assign(this->m_sessionid);
}

template<size_t BufSize>
void trane::Session<BufSize>::handle_cmd_ping(const msgpack::object& obj)
{
    ParamPing param;
    obj.convert(param);
    auto& ping = P0(param);

    std::cout << "Server Received PING(" << ping << ")\n";
    this->send_cmd_pong("PONG");
}

template<size_t BufSize>
trane::Session<BufSize>::Session(asio::io_service& ios, uint64_t sessionid, ErrorHandler eh)
    : Connection<BufSize>(ios, sessionid, eh)
{ }

template<size_t BufSize>
void trane::Session<BufSize>::start()
{
    this->do_read();
};


#endif

