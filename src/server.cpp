#include "../inc/trane.hpp"

template<size_t BufSize>
void foo(const trane::Server<BufSize>& server)
{
    const auto& sessions = server.sessions();
    std::for_each(sessions.entries().begin(), sessions.entries().end(), [](const std::pair<unsigned int, std::shared_ptr<trane::Session<>>>& entry){
        if(entry.second->state() == trane::ConnectionState::CONNECTED)
        {
            std::cout << "Session " << std::setw(16) << std::setfill('0') << std::hex << entry.first << ":\n";
            auto trane_server = asio::ip::address::from_string("127.0.0.1");
            entry.second->create_tunnel(trane_server, trane::TraneType::TCP, "localhost", 22);
        }
    });
}


int main(int argc, char **argv)
{
    unsigned short port = 39999;

    if(argc >= 2)
    {
        std::istringstream iss(std::move(std::string(argv[1])));
        iss >> port;
    }

    asio::io_service ios;
    std::cout << "A\n";
    trane::Server<TRANE_BUFSIZE> server(ios, port);
    std::cout << "B\n";

    std::cout << "Starting Trane Server on 0.0.0.0:" << port << "\n";

    std::thread t([&ios]{
        ios.run();
    });

    while(1)
    {
        std::cout << "Press enter to create tunnel requests.";
        std::cout.flush();
        std::cin.ignore();
        foo(server);
    }

    t.join();

    return 0;
}
