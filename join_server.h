#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <boost/asio/yield.hpp>
using namespace boost::asio;
#define MEM_FN2(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)
class TalkToClient : public boost::enable_shared_from_this<TalkToClient>,
                     coroutine,
                     boost::noncopyable
{
public:
  using ptr = boost::shared_ptr<TalkToClient>;

private:
  using error_code = boost::system::error_code;
  TalkToClient(boost::asio::io_service &io_service) : sock_(io_service), started_(false) {}

public:
  static auto new_(boost::asio::io_service &io_service)
  {
    ptr new_(new TalkToClient(io_service));
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
    sock_.close();
  }
  auto &sock() { return sock_; }
  void do_step(const error_code &err = error_code(), size_t bytes = 0)
  {
    auto self = shared_from_this();
    reenter(this)
    {
      for (;;)
      {
       yield async_read_until( sock_, read_buffer_,"\n", [self](const error_code& err, size_t bytes){
         if(!err) {
           self->do_step();
          return;
         }
          self->stop();
       });

      yield async_write(sock_, write_buffer_,[self](const error_code& err, size_t bytes){
         if(!err) {
           self->do_step();
          return;
         }
          self->stop();
      });
      }
    }
  }

private:
  ip::tcp::socket sock_;
  bool started_;
  streambuf read_buffer_;
  streambuf write_buffer_;
};

class JoinServer : public boost::enable_shared_from_this<JoinServer>, boost::noncopyable
{
private:
  using self_type = JoinServer;
  using error_code = boost::system::error_code;

public:
  JoinServer(unsigned short port_number) : io_service(), acceptor(io_service, ip::tcp::endpoint{ip::tcp::v4(), port_number}), isStarted_(false)
  {
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
    TalkToClient::ptr client = TalkToClient::new_(io_service);
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
    auto new_client = TalkToClient::new_(io_service);
    acceptor.async_accept(new_client->sock(), MEM_FN2(handle_accept, new_client, _1));
  }
  boost::asio::io_service io_service;
  ip::tcp::acceptor acceptor;
  bool isStarted_;
};