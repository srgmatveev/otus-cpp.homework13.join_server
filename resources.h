#pragma once
static const std::string SUCCESS_CODE = "OK";
static const std::string ERROR_CODE = "ERR";

class Cmd_Enum_Map
{
  public:
    static Cmd_Enum_Map &Instance()
    {
        static Cmd_Enum_Map instance;
        return instance;
    }

    auto get_number(const std::string str)
    {
        auto it = cmd_map.find(str);
        if (it != cmd_map.cend())
            return it->second;
        return 0;
    }

  private:
    Cmd_Enum_Map()
    {
        cmd_map["INSERT"] = 1;
        cmd_map["TRUNCATE"] = 2;
    }
    std::map<std::string, int> cmd_map;
};

enum class Cmd
{
    INSERT = 1,
    TRUNCATE,
};
