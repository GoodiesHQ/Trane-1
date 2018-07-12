#include <iostream>
#include <string>
#include <thread>
#include "../inc/trane.hpp"

int main()
{
    asio::io_service ios_server, ios_client;
    trane::Server server(ios_server, 49999);
    std::thread t1([&ios_server](){
        ios_server.run();
    });

    std::cout << "Press enter to continue...\n";
    std::cin.ignore();

    trane::Client client(ios_client, "127.0.0.1", 49999);
    std::thread t2([&ios_client, &client](){
        client.start();
        ios_client.run();
    });

    t1.join();
    t2.join();
    std::cout << "All Done!\n";
    return 0;
}
