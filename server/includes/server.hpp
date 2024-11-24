#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <string>
#include <iostream>

#define ERR "[ERROR]"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
  Session(tcp::socket socket) : socket_(std::move(socket)) {}
  void start(){do_read();}
private:
  void do_read(){
    auto self(shared_from_this());
  boost::asio::async_read_until(
      socket_, boost::asio::dynamic_buffer(read_buf_), '\n',
  [this,self](boost::system::error_code ec, std::size_t l){
        if(ec) {std::cerr << ERR << " Session->do_read(): " <<ec.what() <<std::endl; return ;}
          std::string msg(read_buf_.substr(0,l));
          read_buf_.erase(0,l);
        do_write(msg);
      });
  }

  void do_write(const std::string& msg){
    auto self(shared_from_this());
  boost::asio::async_write(
      socket_, boost::asio::buffer(msg+'\n'),
      [this,self](boost::system::error_code ec,std::size_t /*length*/){
        if(ec) {std::cerr << ERR << "Session->do_write(): "<< ec.what() <<std::endl; return;}
        do_read();
      });
  }
//read buf
std::string read_buf_;
tcp::socket socket_;
};

class Server{
public:
  Server(boost::asio::io_context& io_context, unsigned short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context){do_accept();} 
private:
  void do_accept(){
    acceptor_.async_accept(socket_,
      [this](boost::system::error_code ec) {
        if(ec) {std::cerr << ERR << " Server->do_accept(): " << ec.what() << std::endl; do_accept();}
        std::make_shared<Session>(std::move(socket_))->start();
      });
  }
  tcp::acceptor acceptor_;
  tcp::socket socket_;
};
#endif //SERVER_H
