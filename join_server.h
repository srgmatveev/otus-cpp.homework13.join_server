#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/thread/thread.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <exception>
#include "yield.hpp"
#include "thread_pool.h"
#include "database_cmds.hpp"
#include "resources.h"
using namespace boost::asio;
using namespace boost::system;
#define MEM_FN2(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)
class TalkToClient : public boost::enable_shared_from_this<TalkToClient>,
                     coroutine,
                     boost::noncopyable
{
public:
  using ptr = boost::shared_ptr<TalkToClient>;

private:
  using self_type = TalkToClient;
  using error_code = boost::system::error_code;
  TalkToClient(boost::asio::io_service &io_service, boost::shared_ptr<ThreadPool> tp_ptr) : io_service_(io_service),
                                                                                            sock_(boost::make_shared<ip::tcp::socket>(io_service)), tp_ptr_(tp_ptr), started_(false),
                                                                                            db_cmds_ptr{boost::make_shared<DB_Cmds>(sock_)} {}

public:
  static auto new_(boost::asio::io_service &io_service, boost::shared_ptr<ThreadPool> tp_ptr)
  {
    ptr new_(new TalkToClient(io_service, tp_ptr));
    return new_;
  }
  void start()
  {
    if (started_)
      return;
    started_ = true;
    do_step();
  }
  void stop()
  {
    if (!started_)
      return;
    started_ = false;
    sock_->close();
  }
  auto &sock() { return *sock_; }
  void on_answer_from_server()
  {
    auto self = shared_from_this();

    tp_ptr_->enqueue([self]() {
      try
      {
        bool result{false};
        std::string read_str{""};
        std::getline(std::istream(&self->read_buffer_), read_str);
        result = self->db_cmds_ptr->run_cmd(read_str);
        std::ostream oss(&self->write_buffer_);
        if (result)
        {
          oss << SUCCESS_CODE << std::endl;
        }
        else
        {
          oss << ERROR_CODE << std::endl;
        }
        self->do_step();
      }
      catch (std::exception &e)
      {
        std::ostream oss(&self->write_buffer_);
        oss << ERROR_CODE << " " << e.what() << std::endl;
        self->do_step(errc::make_error_code(errc::state_not_recoverable));
      }
    });
  }
  void do_step(const error_code &err = error_code(), size_t bytes = 0)
  {
    auto self = shared_from_this();
    reenter(this)
    {
      for (;;)
      {

        my_yield async_read_until(*sock_, read_buffer_, "\n", [self](const error_code &err, size_t bytes) {
          if (!err)
          {
            self->do_step();
            return;
          }
          self->stop();
        });
        my_yield io_service_.post([self]() { self->on_answer_from_server(); });
        my_yield async_write(*sock_, write_buffer_, [self](const error_code &err, size_t bytes) {
          if (!err)
          {
            self->do_step();
            return;
          }
          self->stop();
        });
      }
    }
  }

private:
  boost::asio::io_service &io_service_;
  boost::shared_ptr<ip::tcp::socket> sock_;
  bool started_;
  streambuf read_buffer_;
  streambuf write_buffer_;
  boost::shared_ptr<ThreadPool> tp_ptr_{nullptr};
  boost::shared_ptr<DB_Cmds> db_cmds_ptr{nullptr};
};

class JoinServer : public boost::enable_shared_from_this<JoinServer>, boost::noncopyable
{
private:
  using self_type = JoinServer;
  using error_code = boost::system::error_code;

public:
  JoinServer(unsigned short port_number) : io_service(), acceptor(io_service, ip::tcp::endpoint{ip::tcp::v4(), port_number}), isStarted_(false)
  {
    std::size_t max_threads{0};
    max_threads = boost::thread::hardware_concurrency() ? boost::thread::hardware_concurrency() : 1;
    tp_ptr = ThreadPool::createPool(max_threads);
  }
  static auto createServer(unsigned short port_number)
  {
    return boost::make_shared<JoinServer>(port_number);
  }
  void start()
  {
    if (isStarted_)
      return;
    isStarted_ = true;
    TalkToClient::ptr client = TalkToClient::new_(io_service, tp_ptr);
    acceptor.async_accept(client->sock(), MEM_FN2(handle_accept, client, _1));
    io_service.run();
  }
  void stop()
  {
    if (!isStarted_)
      return;
    isStarted_ = false;
    io_service.stop();
  }

  ~JoinServer() { stop(); }

private:
  void handle_accept(TalkToClient::ptr client, const error_code &err)
  {
    client->start();
    auto new_client = TalkToClient::new_(io_service, tp_ptr);
    acceptor.async_accept(new_client->sock(), MEM_FN2(handle_accept, new_client, _1));
  }
  boost::asio::io_service io_service;
  ip::tcp::acceptor acceptor;
  bool isStarted_;
  boost::shared_ptr<ThreadPool> tp_ptr{nullptr};
};