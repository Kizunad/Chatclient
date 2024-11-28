#include "server.hpp"
#include <cstdlib>
#include <iostream>

// ./Server <port>
int main(int argc, char **argv) {

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <port>";
    return 1;
  }

  int port = std::atoi(argv[1]);
  if (port <= 0 || port > 65535) {
    std::cerr << "Invalid port number: " << argv[1] << std::endl;
    return 1;
  }

  std::cout<< "Creating server" <<std::endl;

  try {
    boost::asio::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(),port);
    Server server(io_context, endpoint);
    io_context.run();
  } catch (const std::exception &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}