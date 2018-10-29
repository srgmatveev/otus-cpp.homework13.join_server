#include <exception>
#include <iostream>
#include "utils.h"
#include "join_server.h"
int main(int argc, char const *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Launch parameters:<<" << argv[0] << " <port>" << std::endl
                      << "where <port> - tcp number port" << std::endl;
            return 1;
        }
        if (!is_port<char const *>(argv[1]))
        {
            return 1;
        }
        unsigned short port_number = std::atoi(argv[1]);
        auto server_ptr = JoinServer::createServer(port_number);
        server_ptr->start();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

   return 0;
}
