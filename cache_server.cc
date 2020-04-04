#include "crow_all.h"
#include "cache_lib.cc"
#include "evictor.hh"
#include "cache.hh"
#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>

using namespace crow;

#define LOCALHOST_ADDRESS "127.0.0.1"
#define PORT 8080


void set_header(const crow::request& req, crow::response& res, const Cache& cache) {
  CROW_LOG_INFO << "Server received a HEAD request";

  res.add_header("Space-Used", std::to_string(cache.space_used()));
  res.set_header("Accept", req.get_header_value("accept"));
  res.set_header("Content-Type", "application/json");

  res.end();
}

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
    .methods("HEAD"_method ,"GET"_method, "DELETE"_method)
  ([&cache, &size](const crow::request& req, crow::response& res, key_type key)
  {
    if (req.method == "HEAD"_method) {
      set_header(req, res, cache);
    } else if (req.method =="GET"_method) {
      auto value = cache.get(key, size);
      if (!value) {
        res.code = 404;
        res.write("Key not found");
        res.end();
      } else {
        crow::json::wvalue x;
        x[key] = value;
        res.write(crow::json::dump(x));
        res.end();
      }
    } else {
      //req.method == "DELETE"_method
      cache.del(key);
      res.end();
    }

  });


  // PUT /k/v


  CROW_ROUTE(app, "/<string>/<string>")
    .methods("HEAD"_method, "PUT"_method)
  ([&cache, &size](const crow::request& req, crow::response& res, key_type key, std::string value)
  {
    if (req.method == "HEAD"_method) {
      set_header(req, res, cache);
    }

    Cache::val_type pval = value.c_str();
    size = value.size()+1;
    cache.set(key, pval, size);

    res.end();

  });

  CROW_ROUTE(app, "/reset")
    .methods("HEAD"_method, "POST"_method)
  ([&cache, &size](const crow::request& req, crow::response& res)
  {
    if (req.method == "HEAD"_method) {
      set_header(req, res, cache);
    }
    cache.reset();
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
