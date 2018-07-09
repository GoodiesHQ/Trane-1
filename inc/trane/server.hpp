#ifndef TRANE_SERVER_HPP
#define TRANE_SERVER_HPP
#include "asio_standalone.hpp"
#include "session.hpp"
#include <functional>
#include <random>
#include <unordered_map>

#define SCOPELOCK(mtx) std::lock_guard<std::mutex> __lock(mtx)

namespace trane
{
    /*
     * Trane server wil always use TCP to communicate with clients.
     */
    class Server
    {
    public:
        Server(asio::io_service& ios, unsigned short port);
        std::shared_ptr<Session<>> gen_session();

    protected:
        void handle_accept(std::shared_ptr<Session<>> session, const asio::error_code& err);
        void delete_session(std::uint64_t sessionid);

        unsigned short m_port;
        asio::io_service& m_ios;
        tcp::acceptor m_acceptor;

        std::unordered_map<std::uint64_t, std::shared_ptr<Session<>>> m_sessions;
        std::mutex m_mu;
        std::mt19937_64 m_gen;
    };
}

trane::Server::Server(asio::io_service& ios, unsigned short port)
    : m_port{port}, m_ios{ios}, m_acceptor{ios, tcp::endpoint(tcp::v4(), port)}
{
    std::random_device rd;
    std::array<std::uint_fast64_t, std::mt19937_64::state_size> state;
    std::generate(std::begin(state), std::end(state),
        [&rd](){
            return (static_cast<uint_fast64_t>(rd()) << 32) | rd();
        }
    );
    std::seed_seq seq(std::begin(state), std::end(state));
    m_gen.seed(seq);
    handle_accept(nullptr, asio::error_code());
}

std::shared_ptr<trane::Session<>> trane::Server::gen_session()
{
    SCOPELOCK(m_mu);
    uint64_t sid;
    do
    {
        sid = m_gen();
    }while(m_sessions.find(sid) != m_sessions.end());
    std::shared_ptr<trane::Session<>> session(new Session<>(m_ios, sid, std::bind(&Server::delete_session, this, std::placeholders::_1)));
    m_sessions[sid] = session;
    return session;
}

void trane::Server::handle_accept(std::shared_ptr<trane::Session<>> session, const asio::error_code& err)
{
    if(err)
    {
        std::cerr << "Error: " << err.message() << '\n';
        return;
    }

    if(session != nullptr)
    {
        session->start();
    }

    auto new_session = gen_session();
    std::cout << "Created session.\n";
    m_acceptor.async_accept(
        const_cast<tcp::socket&>(new_session->socket()),
        std::bind(&trane::Server::handle_accept, this, new_session, std::placeholders::_1)
    );
}

void trane::Server::delete_session(uint64_t sessionid)
{
    SCOPELOCK(m_mu);
    m_sessions.erase(sessionid);
}
#endif
