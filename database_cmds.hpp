#pragma once
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "database.h"
#include "resources.h"
using namespace boost::asio;

class DB_Cmds : boost::noncopyable
{
  public:
    DB_Cmds(streambuf &buf) : write_(buf)
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
                return insert();
                break;
            }
            case static_cast<int>(Cmd::TRUNCATE):
            {
                return truncate();
                break;
            }
            case static_cast<int>(Cmd::INTERSECTION):
            {
                return intersect();
                break;
            }
            case static_cast<int>(Cmd::SYMMETRIC_DIFFERENCE):
            {
                return sym_diff();
                break;
            }
            default:
            {
                return unknown_command();
                break;
            }
            }
        }
        return false;
    }

  private:
    bool insert()
    {
        std::ostream out(&write_);
        if (cmds.size() != 4)
        {
            out << ERROR_CODE << " Bad format INSERT string." << std::endl;
            return false;
        }
        auto tbl = Database::Instance().getTable(cmds[1]);
        if (tbl)
        {
            if (tbl->insert(std::stoi(cmds[2]), cmds[3]))
            {
                return true;
            }
            else
            {
                out << ERROR_CODE << " duplicate " << cmds[2] << std::endl;
                return false;
            }
        }
        else
        {
            out << ERROR_CODE << " Table " << cmds[1] << " does't exist." << std::endl;
        }
        return false;
    }

    bool truncate()
    {
        std::ostream out(&write_);
        if (cmds.size() != 2)
        {
            out << ERROR_CODE << " Bad format TRUNCATE table command." << std::endl;
            return false;
        }
        auto tbl = Database::Instance().getTable(cmds[1]);
        if (tbl)
        {
            tbl->truncate();
            return true;
        }
        else
        {
            out << ERROR_CODE << " Table " << cmds[1] << " does't exist." << std::endl;
        }
        return false;
    }
    bool unknown_command()
    {
        std::ostream out(&write_);
        out << ERROR_CODE << " Unknown command." << std::endl;
        return false;
    }
    bool intersect()
    {
        using vect_def = std::vector<std::string>;
        vect_def answer;
        boost::shared_ptr<vect_def> answer_ptr = boost::make_shared<vect_def>(answer);
        std::ostream out(&write_);
        if (cmds.size() != 1)
        {
            out << ERROR_CODE << " Bad format INTERSECTION table command." << std::endl;
            return false;
        }
        auto tblA = Database::Instance().getTable("A");
        if (!tblA)
        {
            out << ERROR_CODE << " Table A doesn't exist." << std::endl;
            return false;
        }
        auto tblB = Database::Instance().getTable("B");
        if (!tblB)
        {
            out << ERROR_CODE << " Table B doesn't exist." << std::endl;
            return false;
        }

        tblA->intersection(*tblB, answer_ptr);
        for (auto &item : *answer_ptr)
        {
            out << item << std::endl;
        }

        return true;
    }
    bool sym_diff()
    {
        std::ostream out(&write_);
        if (cmds.size() != 1)
        {
            out << ERROR_CODE << " Bad format SYMMETRIC_DIFFERENCE table command." << std::endl;
            return false;
        }
        return false;
    }
    streambuf &write_;
    std::vector<std::string> cmds;
};
