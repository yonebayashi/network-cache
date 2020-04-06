#define CATCH_CONFIG_MAIN   // Let Catch provide main():

#include <catch2/catch.hpp>

#include <chrono>
#include <thread>
#include <iostream>
#include <cassert>
#include <typeinfo>
#include <cstring>
#include <stdlib.h>

#include "evictor.hh"
#include "cache.hh"
#include "fifo_evictor.hh"

#include "cache_client.cc"    // TODO: put this in Makefile


#define LOCALHOST_ADDRESS "127.0.0.1"
#define PORT "8080"


TEST_CASE ("Test cache operations over the network") {
  Cache cache(LOCALHOST_ADDRESS, PORT);
  Cache::size_type size;

  const Cache::val_type val1 = "1";
  const Cache::val_type val2 = "2";
  const Cache::val_type val3 = "12";
  const Cache::val_type val4 = "12345";


  SECTION( "should not get any key that isn't in the cache" )
  {
    cache.reset();
    REQUIRE(cache.space_used() == 0);

    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
  }


  SECTION( "should get any keys in the cache" )
  {
    cache.reset();
    REQUIRE(cache.space_used() == 0);

    cache.set("k1", val1, strlen(val1)+1);
    cache.get("k1", size);
    REQUIRE(size == 2);

    cache.set("k3", val3, strlen(val3)+1);
    cache.get("k3", size);
    REQUIRE(size == 3);

    REQUIRE(cache.space_used() == 5);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
  }

  SECTION ( "should update value of a key that was inserted and modified" )
  {
    cache.reset();
    REQUIRE(cache.space_used() == 0);

    cache.set("k1", val1, strlen(val1)+1);
    cache.get("k1", size);
    REQUIRE(size == 2);

    cache.set("k1", val3, strlen(val3)+1);
    cache.get("k1", size);
    REQUIRE(size == 3);

    REQUIRE(cache.space_used() == 3);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
  }


  SECTION( "should update memory used correctly" )
  {
    cache.reset();
    REQUIRE(cache.space_used() == 0);

    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);
    cache.set("k3", val3, strlen(val3)+1);

    cache.get("k1", size);
    REQUIRE(size == 2);

    cache.get("k2", size);
    REQUIRE(size == 2);

    cache.get("k3", size);
    REQUIRE(size == 3);

    REQUIRE(cache.space_used() == 7);

    REQUIRE(cache.del("k1") == true);
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(cache.space_used() == 5);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
  }


  SECTION( "cache should stop accepting new keys when maxmem is exceeded" )
  {
    cache.reset();
    REQUIRE(cache.space_used() == 0);

    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);
    cache.set("k3", val3, strlen(val3)+1);
    cache.set("k4", val4, strlen(val4)+1);

    cache.get("k1", size);
    REQUIRE(size == 2);

    cache.get("k2", size);
    REQUIRE(size == 2);

    cache.get("k3", size);
    REQUIRE(size == 3);

    REQUIRE(cache.get("k4", size) == NULL);   // should not accept k4
    REQUIRE(cache.space_used() == 7);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
  }
}
