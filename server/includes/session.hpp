#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <memory.h>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {

};

#endif