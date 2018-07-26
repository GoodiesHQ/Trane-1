#ifndef TRANE_SERVER_HPP
#define TRANE_SERVER_HPP

#include <functional>
#include <random>
#include <unordered_map>
#include <iostream>
#include <iomanip>

#include "session.hpp"
#include "container.hpp"
#include "random.hpp"
#include "asio_standalone.hpp"
#include "server_proxy.hpp"
#include "logging.hpp"

namespace trane
{
    /*
     * Trane server wil always use TCP to communicate with clients.
     */
    template<size_t BufSize = TRANE_BUFSIZE>
    class Server
    {
    public:
        Server(asio::io_service& ios, uint16_t port);
        std::shared_ptr<Session<BufSize>> gen_session();
        const Container<Session<BufSize>>& sessions() const;
        void listen();

    protected:
        void do_accept();

        void handle_accept(std::shared_ptr<Session<BufSize>> session, const asio::error_code& err);

        void delete_session(std::uint64_t sessionid);

        // constructor initialization list
        uint16_t m_port;
        asio::io_service& m_ios;
        tcp::acceptor m_acceptor;

        // initialized elsewhere
        trane::Random<std::mt19937_64> m_random;
        Container<Session<BufSize>> m_sessions;
    };
}


template<size_t BufSize>
const trane::Container<trane::Session<BufSize>>& trane::Server<BufSize>::sessions() const
{
    return m_sessions;
}


template<size_t BufSize>
void trane::Server<BufSize>::listen()
{
    this->do_accept();
}

template<size_t BufSize>
trane::Server<BufSize>::Server(asio::io_service& ios, uint16_t port)
    : m_port{port}, m_ios{ios}, m_acceptor{ios, tcp::endpoint(tcp::v4(), port)}
{
    LOG(VERBOSE) << "Server Constructor";
}


template<size_t BufSize>
void trane::Server<BufSize>::do_accept()
{
    LOG(INFO) << "Creating a new session";
    auto new_session = gen_session();

    // create a new session and set the current function as the next callback for the next accept() call
    m_acceptor.async_accept(
        const_cast<tcp::socket&>(new_session->socket()),
        std::bind(&trane::Server<BufSize>::handle_accept, this, new_session, std::placeholders::_1)
    );
}


/*
 * Generate a session with a valid session ID. Session will be deleted upon error.
 */
template<size_t BufSize>
std::shared_ptr<trane::Session<BufSize>> trane::Server<BufSize>::gen_session()
{
    auto ptr = std::make_shared<Session<BufSize>>(m_ios, 0, std::bind(&Server::delete_session, this, std::placeholders::_1));
    uint64_t id = m_sessions.add(ptr);
    ptr->set_sessionid(id);
    return ptr;
}


/*
 * Asynchronous handling of accepting sessions. Accept shared ownership of a session object to prevent it from going out
 * of scope.
 */

template<size_t BufSize>
void trane::Server<BufSize>::handle_accept(std::shared_ptr<trane::Session<BufSize>> session, const asio::error_code& err)
{
    if(err)
    {
        // TODO: improve error handling
        LOG(ERROR) << "Accept Error: " << err.message();
        return;
    }

    // if the session is valid start the session.
    if(session != nullptr)
    {
        session->start();
        LOG(DEBUG) << "Staring Session";
    }
    this->do_accept();
}


template<size_t BufSize>
void trane::Server<BufSize>::delete_session(uint64_t sessionid)
{
    LOG(WARNING) << "deleting session " << std::dec << sessionid;
    m_sessions.del(sessionid);
}

#endif
