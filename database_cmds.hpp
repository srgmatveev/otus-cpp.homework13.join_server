#pragma once
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "database.h"
#include "resources.h"
using namespace boost::asio;

class DB_Cmds : public boost::enable_shared_from_this<DB_Cmds>,
                boost::noncopyable
{
  public:
    DB_Cmds(boost::shared_ptr<ip::tcp::socket> sock_ptr) : socket_ptr(sock_ptr)
    {
    }
    bool run_cmd(const std::string str)
    {
        boost::split(cmds, str, boost::is_any_of(" "), boost::token_compress_on);
        if (cmds.size() > 0 && !cmds[0].empty())
        {
            switch (Cmd_Enum_Map::Instance().get_number(cmds[0]))
            {
            case static_cast<int>(Cmd::INSERT):
            {
                return true;
            }    
            case static_cast<int>(Cmd::TRUNCATE):{
                return true;
            }
            default:
                break;
            }
        }
        return false;
    }
    boost::shared_ptr<ip::tcp::socket> socket_ptr;
    std::vector<std::string> cmds;
};
