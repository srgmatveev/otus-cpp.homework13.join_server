
#include <exception>
#include <iostream>
#include "utils.h"
#include "join_server.h"
#include <boost/asio.hpp>
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
        boost::asio::io_service io_service;
        auto server_ptr = JoinServer::createServer(port_number, io_service);
        server_ptr->start();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
