#ifdef TRANE_CLIENT
#include "../inc/trane/client.hpp"

#include <string>
#include <sstream>


const int RECONNECT_INTERVAL = 3;
LogLevel LOGLEVEL = VERBOSE;


void onerror(uint64_t sessionid)
{
    std::cout << "Ending Session ID " << sessionid << '\n';
}


int main(int argc, char **argv)
{
    unsigned short port{39999};

    if(argc != 3 && argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Site Name> <Server Hostname/IP> [port=39999]\n\n";
        return 1;
    }

    if(argc == 4)
    {
        std::string s(argv[3]);
        std::istringstream iss(s);
        iss >> port;
    }

    while(true)
    {
        asio::io_service ios;
        int i;
        trane::Client<TRANE_BUFSIZE> client(ios, argv[1], argv[2], port, &onerror);
        client.start();

        ios.run();

        std::cout << "Connection Error.\n";

        for(i = RECONNECT_INTERVAL; i >= 0; --i)
        {
            std::cout << "\rReconnecting in " << std::dec << i << " seconds...   ";
            std::cout.flush();
            std::this_thread::sleep_for(SEC(1));
        }
    }

    return 0;
}
#endif