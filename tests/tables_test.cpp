#include "gmock/gmock.h"
#include "database.h"
#include <string>
#include <memory>
#include <sstream>
using namespace testing;

class TestTables : public Test
{
public:
  std::shared_ptr<Table> tableA_ptr;
  std::ostringstream oss;
  void SetUp() override
  {
    tableA_ptr = std::make_shared<Table>("A");
  }
};

TEST_F(TestTables, TestTables_Table_Name)
{

  ASSERT_EQ(tableA_ptr->get_name(), "A");
  tableA_ptr->set_name("B");
  ASSERT_EQ(tableA_ptr->get_name(), "B");
  tableA_ptr->set_name("A");
  ASSERT_EQ(tableA_ptr->get_name(), "A");
}

TEST_F(TestTables, TestTables_Insert)
{

  ASSERT_TRUE(tableA_ptr->insert(0, "hello"));
  ASSERT_FALSE(tableA_ptr->insert(0, "hello"));
  ASSERT_TRUE(tableA_ptr->insert(1, "hello1"));
  ASSERT_TRUE(tableA_ptr->insert(3, "hello3"));
  ASSERT_TRUE(tableA_ptr->insert(2, "hello2"));
}

TEST_F(TestTables, TestTables_Sorted)
{

  std::string resultData{
      "0 hello0\n"
      "1 hello1\n"
      "2 hello2\n"
      "3 hello3\n"};
  tableA_ptr->insert(3, "hello3");
  tableA_ptr->insert(2, "hello2");
  tableA_ptr->insert(1, "hello1");
  tableA_ptr->insert(0, "hello0");
  for (auto &elem : tableA_ptr->get_table())
  {
    oss << elem.first << " " << elem.second << std::endl;
  }
  ASSERT_THAT(oss.str(), resultData);
}