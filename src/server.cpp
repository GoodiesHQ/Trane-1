#include "../inc/trane.hpp"

void foo(const trane::Server& server)
{
    const auto& sessions = server.sessions();
    std::for_each(sessions.begin(), sessions.end(), [](const std::pair<unsigned int, std::shared_ptr<trane::Session<>>>& entry){
        if(entry.second->state() == trane::ConnectionState::CONNECTED)
        {
            std::cout << "Session " << std::setw(16) << std::setfill('0') << std::hex << entry.first << ":\n";
            entry.second->add_request(std::make_tuple("127.0.0.1", 1234, "127.0.0.1", 4321, 0x1122334455667788ULL));
        }
    });
}


int main(int argc, char **argv)
{
    unsigned short port = 49999;

    if(argc >= 2)
    {
        std::istringstream iss(std::move(std::string(argv[1])));
        iss >> port;
    }

    asio::io_service ios;
    trane::Server server(ios, 49999);

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
