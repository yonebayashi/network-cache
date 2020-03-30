#include "crow_all.h"
#include "cache_lib.cc"
#include "evictor.hh"
#include "cache.hh"
#include <iostream>

using namespace crow;

#define LOCALHOST_ADDRESS "127.0.0.1"
#define PORT 8080


int main(int argc, char *argv[])
{
  // TODO: handle optional command line arguments

  SimpleApp app;
  CROW_ROUTE(app, "/")([]{
    return "Hello world\n";
  });


  Cache cache(8, 0.75, nullptr);
  Cache::size_type size;

  const Cache::val_type val1 = "1";
  const Cache::val_type val2 = "2";

  cache.set("k1", val1, strlen(val1)+1);
  cache.set("k2", val2, strlen(val2)+1);

  // GET /key
  // test with "curl -X GET http://localhost:8080/{key} --output -"
  CROW_ROUTE(app, "/<string>")
    .methods("GET"_method)
  ([&cache, &size](key_type key){
    crow::json::wvalue x;
    auto value = cache.get(key, size);
    if (!value)
      return crow::response(404, "Key not found");
    x[key] = value;
    return crow::response(x);
  });

  // PUT /k/v


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
