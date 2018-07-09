#include <iostream>
#include <string>
#include <thread>
#include "../inc/trane.hpp"

int main()
{
    asio::io_service ios;
    trane::Server server(ios, 49999);
    std::thread t([&ios](){
        ios.run();
    });
    t.join();
    std::cout << "All Done!\n";
    return 0;
}
