#pragma once
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <iterator>
#include <memory>
#include <shared_mutex>
#include <boost/shared_ptr.hpp>
#include <sstream>
class Table
{
    using pair_type = std::pair<int, std::string>;
    using vect_def = std::vector<std::string>;

  public:
    Table(const std::string &str) : name_(str) {}
    bool insert(int a, const std::string &str)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = structure_.find(a);
        if (it == structure_.cend())
        {
            structure_.emplace(pair_type(a, str));
            return true;
        }
        return false;
    }
    void truncate()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        structure_.clear();
    }

    void intersection(const Table &other, boost::shared_ptr<vect_def> answer_ptr)
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::stringstream ss;
        auto first1 = std::cbegin(structure_);
        auto last1 = std::cend(structure_);
        auto first2 = std::cbegin(other.structure_);
        auto last2 = std::cend(other.structure_);
        while (first1 != last1 && first2 != last2)
        {
            if (first1->first < first2->first)
            {
                ++first1;
            }
            else if (first2->first < first1->first)
            {
                ++first2;
            }
            else
            {
                ss.str("");
                ss.clear();
                ss << first1->first << ",";
                ss << first1->second << "," << first2->second;
                answer_ptr->emplace_back(ss.str());
                ++first1;
                ++first2;
            }
        }
    }

    const std::string add_sym_diff_str(int id, const std::string str1, const std::string str2)
    {
        std::stringstream ss;
        ss << id << "," << str1 << "," << str2;
        return ss.str();
    }
    void sym_diff(const Table &other, boost::shared_ptr<vect_def> answer_ptr)
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it_A = std::cbegin(structure_);
        auto lim_A = std::cend(structure_);
        auto it_B = std::cbegin(other.structure_);
        auto lim_B = std::cend(other.structure_);
        while (!(it_A == lim_A && it_B == lim_B))
        {
            if (it_A == lim_A)
            {
                answer_ptr->emplace_back(add_sym_diff_str(it_B->first, "", it_B->second));
                ++it_B;
            }
            else if (it_B == lim_B)
            {
                answer_ptr->emplace_back(add_sym_diff_str(it_A->first, it_A->second, ""));
                ++it_A;
            }
            else if (it_A->first < it_B->first)
            {
                answer_ptr->emplace_back(add_sym_diff_str(it_A->first, it_A->second, ""));
                ++it_A;
            }
            else if (it_B->first < it_A->first)
            {
                answer_ptr->emplace_back(add_sym_diff_str(it_B->first, "", it_B->second));
                ++it_B;
            }
            else
            {
                ++it_A;
                ++it_B;
            }
        }
    }
    const std::string &get_name()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return name_;
    }
    void set_name(const std::string &str)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        name_ = str;
    }

    auto &get_table()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return structure_;
    }

  private:
    std::map<int, std::string> structure_;
    std::string name_;
    std::shared_mutex mutex_;
};

class Database
{
    using pair_type = std::pair<std::string, std::shared_ptr<Table>>;

  public:
    static Database &Instance()
    {
        static Database instance;
        return instance;
    }
    std::shared_ptr<Table> getTable(const std::string &s)
    {
        auto it = tables_ptr_.find(s);
        if (it != tables_ptr_.cend())
            return it->second;
        return nullptr;
    }

  private:
    Database()
    {
        tables_ptr_.insert(pair_type{"A", std::make_shared<Table>("A")});
        tables_ptr_.insert(pair_type{"B", std::make_shared<Table>("B")});
    }
    std::map<std::string, std::shared_ptr<Table>> tables_ptr_;
};