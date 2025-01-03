#include "client.hpp"
#include <cstdio>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <thread>
#include <memory.h>

std::mutex cout_mutex;

void safe_cout(const std::string str){
  std::lock_guard<std::mutex> lock(cout_mutex);
  std::cout << str << std::endl;
}
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
          std::string msg("Recv: "+ client->read_with_wait());
          safe_cout(msg);
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
      safe_cout("Sent: " + buf);
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
