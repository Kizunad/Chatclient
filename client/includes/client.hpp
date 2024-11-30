#ifndef CLIENT_H
#define CLIENT_H

#include <boost/system/detail/error_code.hpp>
#define ERR "[ERROR]"

#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/impl/read.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client> {
public:
  Client(boost::asio::io_context &io_context,
         const tcp::resolver::results_type &endpoints)
      : socket_(io_context), strand_(boost::asio::make_strand(io_context)),
        endpoints_(endpoints) {}

  void start() {
    auto self(shared_from_this());
    do_connect();
  };
  void stop() { do_close(); }
  void write(const std::string &str) {
    {
      std::lock_guard<std::mutex> lock(write_mutex_);
      write_queue_.push_back(str);
    }
    boost::asio::post(strand_, [this]() { do_write(); });
  }

  // optional, if read_queue_ is empty, return nullopt. If there is a value,
  // return string.
  std::optional<std::string> read() {
    std::lock_guard<std::mutex> lock(read_mutex_);
    if (read_queue_.empty())
      return std::nullopt;
    std::string msg = read_queue_.front();
    read_queue_.pop_front();
    return msg;
  }

  // optimized read with lock to block the thread and wait for the message.
  std::string read_with_wait() {
    std::unique_lock<std::mutex> lock(read_mutex_);
    read_cv_.wait(lock, [this]() { return !read_queue_.empty(); });
    std::string msg = read_queue_.front();
    read_queue_.pop_front();
    return msg;
  }

private:
  void read_ec(boost::system::error_code ec){
    if(ec == boost::asio::error::eof){
      std::cerr << "[INFO] Connection closed by server." << std::endl;
    } else {
      std::cerr << ERR << " client do_read() " << ec.what() << std::endl;
    }

  }
  void do_close() {
    auto self(shared_from_this());
    socket_.close();
  }
  void do_connect() {
    auto self(shared_from_this());
    boost::asio::async_connect(
        socket_, endpoints_,
        [this, self](boost::system::error_code ec, tcp::endpoint) {
          if (!ec) {
            std::cout << "Client connected, calling do_read()..." << std::endl;
            boost::asio::post(strand_, [this]() { do_read(); });
          } else {
            std::cerr << ERR << " Client failed to connect. " << ec.what()
                      << std::endl;
          }
        });
  }

  void do_write() {
    if (write_queue_.empty())
      return;

    auto self(shared_from_this());
    std::string msg = write_queue_.front() + '\n';

    boost::asio::async_write(
        socket_, boost::asio::buffer(msg),
        [this, self](boost::system::error_code ec, std::size_t) {
          if (ec) {
            std::cerr << ERR << " do_write(): " << ec.what() << std::endl;
            do_close();
          }
          {
            std::lock_guard<std::mutex> lock(write_mutex_);
            write_queue_.pop_front();
          }
          if (!write_queue_.empty()) {
            boost::asio::post(strand_, [this]() { do_write(); });
          }
        });
  }

  void do_read() {
    auto self(shared_from_this());
    // params:stream, buf, delimiter, return handle/ token
    boost::asio::async_read_until(
        socket_, boost::asio::dynamic_buffer(read_buf_), '\n',
        boost::asio::bind_executor(
            strand_, [this, self](boost::system::error_code ec, std::size_t l) {
              if (ec) {
                read_ec(ec);
                return;
              }
              std::string msg(read_buf_.substr(0, l));
              read_buf_.erase(0, l);

              {
                std::lock_guard<std::mutex> lock(read_mutex_);
                read_queue_.push_back(msg);
              }
              read_cv_.notify_one();
              boost::asio::post(strand_, [this]() { do_read(); });
            }));
  }

  // connection variables
  tcp::socket socket_;
  tcp::resolver::results_type endpoints_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;

  // read variables
  std::string read_buf_;
  std::deque<std::string> read_queue_;

  // write variables
  std::deque<std::string> write_queue_;

  // lock
  std::condition_variable read_cv_;
  std::mutex read_mutex_;
  std::mutex write_mutex_;
};

#endif
