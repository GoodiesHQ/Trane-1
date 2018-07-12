#include "../inc/trane.hpp"

void foo(const trane::Server& server)
{
    const auto& sessions = server.sessions();
    std::for_each(sessions.begin(), sessions.end(), [](const std::pair<unsigned int, std::shared_ptr<trane::Session<>>>& entry){
        std::cout << "Session " << std::setw(16) << std::setfill('0') << entry.first << ":\n";
        entry.second->add_request(std::make_tuple("127.0.0.1", 1234, "127.0.0.1", 4321, 0x1122334455667788ULL));
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

    std::this_thread::sleep_for(SEC(30));
    foo(server);
    t.join();

    return 0;
}
