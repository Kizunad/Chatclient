#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;


class Session : public std::enable_shared_from_this<Session> {
public:
Session(tcp::socket socket) : socket_(std::move(socket)) {}

void start(){do_read();}

void write(const std::string& msg){}

private:
void do_read() {
    auto self(shared_from_this());

    boost::asio::async_read_until(
        socket_,
        boost::asio::dynamic_buffer(data_),
        '\n',
        [this,self](boost::system::error_code ec, std::size_t length){
            if(!ec){
                std::string msg(data_.substr(0,length));
                data_.erase(0,length);

                std::cout << msg << std::endl;
                do_write(msg);
            }});
}

void do_write(const std::string& msg){
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(msg+'\n'),
        [this,self](boost::system::error_code ec, std::size_t){
            if(!ec) {do_read();}
        });
}
tcp::socket socket_;
std::string data_;
};

class Server{
    public:
    Server(boost::asio::io_context& ioc, const tcp::endpoint& endpoint) : acceptor_(ioc,endpoint){do_accept();}
    private:
    void do_accept(){
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket){
                if(!ec) {std::make_shared<Session>(std::move(socket))->start();}
                do_accept();
            });
            }
    tcp::acceptor acceptor_;
};





#endif //SERVER_H