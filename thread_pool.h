#pragma once
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

class ThreadPool
{
    typedef std::unique_ptr<boost::asio::io_service::work> asio_worker;

  public:
    ThreadPool(std::size_t threads) : service_(), working(new asio_worker::element_type(service_)), is_running(true)
    {
        while (threads--)
        {
            auto worker = boost::bind(&boost::asio::io_service::run, &service_);
            g.add_thread(new boost::thread(worker));
        }
    }

    static auto createPool(std::size_t threads)
    {
        return boost::make_shared<ThreadPool>(threads);
    }
    template <class F>
    void enqueue(F f)
    {
        service_.post(f);
    }
    void stop()
    {

        if (!is_running)
            return;
        is_running = false;
        working.reset(); //allow run() to exit
        service_.stop();
        g.join_all();
    }
    ~ThreadPool()
    {
        stop();
    }

  private:
    bool is_running{false};
    boost::asio::io_service service_; //< the io_service we are wrapping
    asio_worker working;
    boost::thread_group g; //< need to keep track of threads so we can join them
};
