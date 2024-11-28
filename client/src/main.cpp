#include "client.hpp"
#include <cstdio>
#include <exception>
#include <iostream>
#include <sys/types.h>
#include <thread>

// ./client <host> <port>
int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  boost::asio::io_context io_context;

  try {
    tcp::resolver resolver(io_context);
    // resolver.resolve(host,port) -> returns endpoints
    auto client = std::make_shared<Client>(io_context,
                                           resolver.resolve(argv[1], argv[2]));

    client->start();

    std::cout << "Starting read_thread" << std::endl;

    std::thread io_thread([&io_context]() { io_context.run(); });
    std::thread read_thread([client]() {
      while (true) {
        try {
          std::cout << "Recv: " << client->read_with_wait() << std::endl;
        } catch (const std::exception &e) {
          std::cerr << "Caught exception: " << e.what() << std::endl;
          break;
        } catch (...) {
          std::cerr << "Unknown exception: " << std::endl;
          break;
        }
      }
    });

    std::string buf;
    while (std::getline(std::cin, buf)) {
      if (buf == ":q") {
        break;
      }
      client->write(buf);
      std::cout << "Sent: " << buf << std::endl;
    }
    client->stop();
    io_context.stop();
    io_thread.join();
    read_thread.join();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
