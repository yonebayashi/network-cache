#include "crow_all.h"
#include <iostream>

using namespace crow;

#define LOCALHOST_ADDRESS "127.0.0.1"
#define PORT 8080


int main(int argc, char *argv[])
{
  // TODO: handle optional command line arguments

  SimpleApp app;
  CROW_ROUTE(app, "/")([]{
    return "Hello world";
  });

  auto _ = async(std::launch::async, [&]{app.bindaddr(LOCALHOST_ADDRESS).port(PORT).run();});

  asio::io_service is;
    {
        asio::ip::tcp::socket c(is);
        c.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(LOCALHOST_ADDRESS), PORT));

        try
        {
        }
        catch(std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

  // app.stop();
  return 0;
}
