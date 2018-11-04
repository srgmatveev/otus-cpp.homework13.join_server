#include "gmock/gmock.h"
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <atomic>
#include <chrono>
#include <vector>
#include <algorithm>
#ifdef WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "resources.h"
using namespace testing;
using namespace std::literals;
using namespace boost::asio;

class TestServer : public Test
{
  public:
    using socket = boost::asio::ip::tcp::socket;
    boost::filesystem::path full_path;
    std::string srv_run_path{};
    std::string srv_name = "join_server";
    std::string port = "9000";
    std::atomic_bool isServerRunning{false};
    std::thread serverThread;
    std::string out_file = "out.txt";
    streambuf read_buf;
    std::vector<std::string> server_response;
    void server_run()
    {
        isServerRunning = false;
        srv_run_path = "";
        if (!boost::filesystem::exists(srv_name))
            chdir("../");
        full_path = boost::filesystem::current_path();
        full_path += "/" + srv_name;
        if (!boost::filesystem::exists(full_path))
            full_path = "";

        if (!full_path.empty())
        {
            srv_run_path = full_path.string() + " " + port;
            server_run_prep();
        }
    }
    void server_run_prep()
    {
        if (isServerRunning)
            return;
        serverThread = std::thread(&TestServer::startServer, this, srv_run_path, out_file, std::ref(isServerRunning));
        while (!isServerRunning)
        {
        };
        std::this_thread::sleep_for(200ms);
    }

    void server_stop()
    {
        if (!isServerRunning)
            return;
        isServerRunning = false;
        std::this_thread::sleep_for(200ms);
        std::system("killall join_server");
        if (serverThread.joinable())
            serverThread.join();
    }
    void startServer(std::string srv_path, std::string out_file, std::atomic_bool &flag)
    {
        if (!out_file.empty())
            srv_path += " >" + out_file;
        flag = true;
        try
        {
            std::system(srv_path.c_str());
        }
        catch (...)
        {
        }
    }
    auto client_start()
    {
        boost::asio::io_service io_service;
        ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 9000);
        auto socket_ptr = boost::make_shared<socket>(io_service);
        socket_ptr->connect(ep);
        return socket_ptr;
    }

    void send_command(boost::shared_ptr<socket> sock,
                      streambuf &buf, const std::string &command,
                      std::vector<std::string> &server_response)
    {
        std::vector<std::string> cmds;
        std::ostream out(&buf);
        out << command << std::endl;
        sock->write_some(buf.data());
        std::string line = "";
        while (true)
        {
            boost::asio::read_until(*sock, buf, '\n');
            std::istream in(&buf);
            while (std::getline(in, line))
            {
                boost::split(cmds, line, boost::is_any_of(" "), boost::token_compress_on);
                server_response.emplace_back(line);
            }

            if (cmds[0] == SUCCESS_CODE || cmds[0] == ERROR_CODE)
                break;
        }
    }
};

TEST_F(TestServer, TestServer_Running)
{
    server_run();
    std::cout << "srv_run_path" << srv_run_path << std::endl;
    ASSERT_TRUE(srv_run_path.empty() ? 0 : 1);
    auto sock = client_start();
    sock->close();
    server_stop();
}
TEST_F(TestServer, TestServer_Insert)
{
    std::vector<std::string> cmds;
    server_run();
    std::cout << "srv_run_path" << srv_run_path << std::endl;
    ASSERT_TRUE(srv_run_path.empty() ? 0 : 1);
    auto sock = client_start();
    send_command(sock, read_buf, "INSERT A 0 lean", server_response);
    EXPECT_THAT(server_response.back(), SUCCESS_CODE);
    send_command(sock, read_buf, "INSERT A 0 lean", server_response);
    boost::split(cmds, server_response.back(), boost::is_any_of(" "), boost::token_compress_on);
    EXPECT_THAT(cmds[0], ERROR_CODE);
    send_command(sock, read_buf, "INSERT A 1 sweater", server_response);
    EXPECT_THAT(server_response.back(), SUCCESS_CODE);
    sock->close();
    server_stop();
}