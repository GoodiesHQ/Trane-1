#ifndef TRANE_COMMANDS_HPP
#define TRANE_COMMANDS_HPP

#include "asio_standalone.hpp"
#include "utils.hpp"
#include <msgpack.hpp>
#include <iostream>
#include <vector>

namespace trane{

    using command_t = std::tuple<unsigned char, msgpack::object>;

    using ParamConnect = std::tuple<std::string>;
    using ParamAssign = std::tuple<uint64_t>;
    using ParamPing = std::tuple<std::string>;
    using ParamPong = std::tuple<std::string>;
    using ParamTunnelReq = std::tuple<std::string, uint16_t, std::string, uint16_t, unsigned char, uint64_t>;
    using ParamTunnelRes = std::tuple<uint64_t, bool, std::string>;

    /*
     * Sequence generator used for parameter unpacking
     */
    template<int...> struct seq {};
    template<int N, int... S> struct gen_seq : gen_seq<N-1, N-1, S...> {};
    template<int... S> struct gen_seq<0, S...> { typedef seq<S...> type; };


    template<typename... Args>
    void serialize_to_buffer(msgpack::sbuffer& buf, Args&&... args)
    {
        msgpack::pack(buf, std::make_tuple(std::forward<Args>(args)...));
    }


    template<typename... Args>
    msgpack::object_handle serialize(msgpack::sbuffer& buf, Args&&... args)
    {
        serialize_to_buffer(buf, std::forward<Args>(args)...);
        msgpack::object_handle handle = msgpack::unpack(buf.data(), buf.size());
        return handle;
    }


    template<typename... Args>
    void create_command(TraneCommand cmd, msgpack::sbuffer &buf, Args&&... args)
    {
        msgpack::sbuffer tmp;
        auto handle = serialize(tmp, std::forward<Args>(args)...);
        auto obj = handle.get();
        msgpack::pack(buf, std::make_tuple(static_cast<unsigned char>(cmd), obj));
    }


    void cmd_connect(msgpack::sbuffer& buf, const std::string& site_name)
    {
        create_command(CONNECT, buf, site_name);
    }


    void cmd_assign(msgpack::sbuffer& buf, uint64_t sessionid)
    {
        create_command(ASSIGN, buf, sessionid);
    }


    void cmd_ping(msgpack::sbuffer& buf, const std::string& message)
    {
        create_command(PING, buf, message);
    }


    void cmd_pong(msgpack::sbuffer& buf, const std::string& message)
    {
        create_command(PONG, buf, message);
    }


    void cmd_tunnel_req(msgpack::sbuffer& buf,
                        const std::string& host_server, uint16_t port_server,
                        const std::string& host_client, uint16_t port_client,
                        unsigned char trane_type, uint64_t tunnelid)
    {
        create_command(TUNNEL_REQ, buf, host_server, port_server, host_client, port_client, trane_type, tunnelid);
    }


    void cmd_tunnel_res(msgpack::sbuffer& buf, uint64_t tunnelid, bool success, const std::string& message)
    {
        create_command(TUNNEL_RES, buf, tunnelid, success, message);
    }
}

#endif
