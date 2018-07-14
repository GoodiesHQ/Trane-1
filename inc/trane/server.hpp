#ifndef TRANE_SERVER_HPP
#define TRANE_SERVER_HPP

#include "utils.hpp"
#include "asio_standalone.hpp"
#include "session.hpp"
#include <functional>
#include <random>
#include <unordered_map>
#include <iostream>
#include <iomanip>

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
        const std::unordered_map<std::uint64_t, std::shared_ptr<Session<>>>& sessions() const;

    protected:
        void handle_accept(std::shared_ptr<Session<>> session, const asio::error_code& err);
        void delete_session(std::uint64_t sessionid);

        // constructor initialization list
        unsigned short m_port;
        asio::io_service& m_ios;
        tcp::acceptor m_acceptor;

        // initialized elsewhere
        std::unordered_map<std::uint64_t, std::shared_ptr<Session<>>> m_sessions;               // Store a collection of each session ID and its respective session
        std::mutex m_mu;                                                                        // mutex designed for use when accessing m_sessions.
        std::mt19937_64 m_gen;                                                                  // mersenne twister

    };
}


const std::unordered_map<std::uint64_t, std::shared_ptr<trane::Session<>>>& trane::Server::sessions() const
{
    return m_sessions;
}

trane::Server::Server(asio::io_service& ios, unsigned short port)
    : m_port{port}, m_ios{ios}, m_acceptor{ios, tcp::endpoint(tcp::v4(), port)}
{
    // most secure initialization... honestly, it's overkill, but might re-use later for something more interesting
    std::random_device rd;
    std::array<std::uint_fast64_t, std::mt19937_64::state_size> state;  // store the random seed state with "optimized" 64 bit ints
    std::generate(std::begin(state), std::end(state),
        [&rd](){
            // random_device::operator() yields 32 bits. Shift + Or makes it 64 bits of randomness.
            return (static_cast<uint_fast64_t>(rd()) << 32) | rd();
        }
    );
    // create the seed sequence with const iterators of the seed state array
    std::seed_seq seq(std::cbegin(state), std::cend(state));
    m_gen.seed(seq); // seed the generator
    handle_accept(nullptr, asio::error_code()); // dummy call to start the process
}

/*
 * Generate a session with a valid session ID. Session will be deleted upon error.
 */
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


/*
 * Asynchronous handling of accepting sessions. Accept shared ownership of a session object to prevent it from going out
 * of scope.
 */
void trane::Server::handle_accept(std::shared_ptr<trane::Session<>> session, const asio::error_code& err)
{
    if(err)
    {
        // TODO: improve debugging
        std::cerr << "Error: " << err.message() << '\n';
        return;
    }

    // if the session is valid (i.e. not the dummy call from the instructor), start the session.
    if(session != nullptr)
    {
        session->start();
    }

    auto new_session = gen_session();
    // create a new session and set the current function as the next callback for the next accept() call
    m_acceptor.async_accept(
        const_cast<tcp::socket&>(new_session->socket()),
        std::bind(&trane::Server::handle_accept, this, new_session, std::placeholders::_1)
    );
}

void trane::Server::delete_session(uint64_t sessionid)
{
    // lock the mutex when accessing/editing the sessions variable
    SCOPELOCK(m_mu);
    auto& session = m_sessions[sessionid];
    const_cast<tcp::socket&>(session->socket()).close();
    m_sessions.erase(sessionid);
}
#endif
