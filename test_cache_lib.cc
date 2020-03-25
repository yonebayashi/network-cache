// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <iostream>
#include <cassert>
#include <typeinfo>
#include <cstring>
#include <stdlib.h>

#include "evictor.hh"
#include "cache.hh"
#include "fifo_evictor.hh"

#include "cache_lib.cc"   // for testing purpose
#include "fifo_evictor.cc"  // for testing purpose



TEST_CASE ("Cache with nullptr Evictor", "[cache]") {
  Cache cache(8, 0.75, nullptr);
  REQUIRE(cache.space_used() == 0);

  const Cache::val_type val1 = "1";
  const Cache::val_type val2 = "2";
  const Cache::val_type val3 = "16";
  const Cache::val_type val4 = "17";

  Cache::size_type size;

  SECTION( "get should not retreive a key that wasn't inserted" ) {
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);
  }

  SECTION ("Cache should handle 100 insertions without crashing") {
    for (unsigned i = 0 ; i<100; i++) {
      cache.set({(char)(i+1), '\0'},val1, size);
    }
  }

  SECTION ("Cache should handle 100 deletions without crashing") {
    for (unsigned i = 0 ; i<100; i++) {
      cache.set({(char)(i+1), '\0'}, val1, size);
    }
    for (unsigned i = 0 ; i<100; i++) {
      cache.del({(char)(i+1), '\0'});
    }
  }

  SECTION( "get should retreive a key that was inserted (with 1 item in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);

    REQUIRE(strcmp(cache.get("k1", size), val1) == 0);   // string comparision
    REQUIRE(size == strlen(val1)+1);

  }

    SECTION( "get should retreive a key that was inserted (with 2 items in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);

    REQUIRE(strcmp(cache.get("k2", size), val2) == 0);
    REQUIRE(size == strlen(val2)+1);

  }

  SECTION( "get should retreive a key that was inserted (with 3 items in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);

    cache.set("k2", val2, strlen(val2)+1);


    cache.set("k3", val3, strlen(val3)+1);

    REQUIRE(strcmp(cache.get("k3", size), val3) == 0);
    REQUIRE(size == strlen(val3)+1);
    REQUIRE(cache.space_used() == 7);


  }

  SECTION("space_used() responds appropriately to new insertions") {
    cache.set("k1", val1, strlen(val1)+1);
    REQUIRE(cache.space_used()==2);

    cache.set("k2", val2, strlen(val2)+1);
    REQUIRE(cache.space_used()==4);

    cache.set("k3", val3, strlen(val3)+1);
    REQUIRE(cache.space_used()==7);

  }

  SECTION ("Eviction policy with nullptr evictor should not allow an insertion that would cause an overflow.") {
    cache.set("k1", val1, strlen(val1)+1);

    cache.set("k2", val2, strlen(val2)+1);


    cache.set("k3", val3, strlen(val3)+1);


    REQUIRE(cache.space_used()==7);

    cache.set("k4", val4, strlen(val4)+1);

    REQUIRE(cache.get("k4", size) == NULL);
    REQUIRE(size == 0);
    REQUIRE(cache.space_used() == 7);
  }


  SECTION( "get should retreive a key that was inserted and modified" ) {
    cache.set("k3", val3, strlen(val3)+1);

    REQUIRE(strcmp(cache.get("k3", size), val3) == 0);
    REQUIRE(size == strlen(val3)+1);
    REQUIRE(cache.space_used() == size);

    // modify value of key 3
    cache.set("k3", val4, strlen(val4)+1);

    REQUIRE(strcmp(cache.get("k3", size), val4) == 0);
    REQUIRE(size == strlen(val4)+1);
    REQUIRE(cache.space_used() == strlen(val4)+1);
  }

  SECTION( "get should not retreive a key that was inserted and deleted" ) {
    cache.set("k1", val1, strlen(val1)+1);

    REQUIRE(strcmp(cache.get("k1", size), val1) == 0);   // string comparision
    REQUIRE(size == strlen(val1)+1);
    REQUIRE(cache.space_used() == 2);

    cache.del("k1");
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);
    REQUIRE(cache.space_used() == 0);
  }

  SECTION( "resetting should return empty cache" ) {
    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
    REQUIRE(cache.get("k1", size)== NULL);
    REQUIRE(cache.get("k2", size)== NULL);
  }
}


TEST_CASE ("Cache with FIFO Evictor", "[evictor]") {
  FifoEvictor* evictor= new FifoEvictor();
  Cache cache(8, 0.75, evictor);
  REQUIRE(cache.space_used() == 0);

  const Cache::val_type val1 = "1";
  const Cache::val_type val2 = "2";
  const Cache::val_type val3 = "16";
  const Cache::val_type val4 = "17";

  Cache::size_type size;

  SECTION ("FIFO evictor can handle 100 key touches without crashing") {
    for (unsigned i = 1 ; i<101; i++) {
      evictor->touch_key({(char)(i), '\0'});
    }
  }

  SECTION ("FIFO evictor displays FIFO functionality with unique keys") {
    for (unsigned i = 1 ; i<101; i++) {
      evictor->touch_key({(char)(i), '\0'});
    }
    char expected_str[] ="X";
    for (unsigned i=100; i>0; i--) {
      expected_str[0]= (char)i;
      REQUIRE(evictor-> evict()==expected_str);
    }
  }

  SECTION ("FIFO evictor displays FIFO functionality with nonunique keys") {

    for (unsigned i = 1 ; i<101; i++) {
      evictor->touch_key({(char)(i), '\0'});
    }
    for (unsigned i = 1 ; i<101; i++) {
      evictor->touch_key({(char)(i), '\0'});
    }
    char expected_str[] ="X";
    for (unsigned i=100; i>0; i--) {
      expected_str[0]= (char)i;
      REQUIRE(evictor-> evict()==expected_str);
    }
    for (unsigned i=100; i>0; i--) {
      expected_str[0]= (char)i;
      REQUIRE(evictor-> evict()==expected_str);
    }
  }


  SECTION( "get should not retreive a key that wasn't inserted" ) {
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);
  }

  SECTION ("Cache should handle 100 insertions without crashing") {
    for (unsigned i = 0 ; i<100; i++) {
      cache.set({(char)(i+1), '\0'},val1, size);
    }
    REQUIRE(size == 2);
  }

  SECTION ("Cache should handle 100 deletions without crashing") {
    for (unsigned i = 0 ; i<100; i++) {
      cache.set({(char)(i+1), '\0'}, val1, size);
    }
    for (unsigned i = 0 ; i<100; i++) {
      cache.del({(char)(i+1), '\0'});
    }
  }
  SECTION( "get should retreive a key that was inserted (with 1 item in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);

    REQUIRE(strcmp(cache.get("k1", size), val1) == 0);   // string comparision
    REQUIRE(size == strlen(val1)+1);

  }

    SECTION( "get should retreive a key that was inserted (with 2 items in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);

    REQUIRE(strcmp(cache.get("k2", size), val2) == 0);
    REQUIRE(size == strlen(val2)+1);

  }

  SECTION( "get should retreive a key that was inserted (with 3 items in cache)" ) {
    cache.set("k1", val1, strlen(val1)+1);

    cache.set("k2", val2, strlen(val2)+1);


    cache.set("k3", val3, strlen(val3)+1);

    REQUIRE(strcmp(cache.get("k3", size), val3) == 0);
    REQUIRE(size == strlen(val3)+1);
    REQUIRE(cache.space_used() == 7);


  }

  SECTION("space_used() responds appropriately to new insertions") {
    cache.set("k1", val1, strlen(val1)+1);
    REQUIRE(cache.space_used()==2);

    cache.set("k2", val2, strlen(val2)+1);
    REQUIRE(cache.space_used()==4);

    cache.set("k3", val3, strlen(val3)+1);
    REQUIRE(cache.space_used()==7);

  }

  SECTION( "Inserting works with FIFO eviction" ) {
    cache.set("k1", val1, strlen(val1)+1);

    cache.set("k2", val2, strlen(val2)+1);

    cache.set("k3", val3, strlen(val3)+1);

    cache.set("k4", val4, strlen(val4)+1);

    // Testing that k4 was successfully inserted
    REQUIRE(strcmp(cache.get("k4", size), val4) == 0);
    REQUIRE(size == strlen(val4)+1);

    // key 1 should be evicted
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);

    // total memory used is occupied by key 2->4
    REQUIRE(cache.space_used() == 8);
  }

  SECTION( "get should retreive a key that was inserted and modified" ) {
    cache.set("k3", val3, strlen(val3)+1);

    REQUIRE(strcmp(cache.get("k3", size), val3) == 0);
    REQUIRE(size == strlen(val3)+1);
    REQUIRE(cache.space_used() == size);

    // modify value of key 3
    cache.set("k3", val4, strlen(val4)+1);

    REQUIRE(strcmp(cache.get("k3", size), val4) == 0);
    REQUIRE(size == strlen(val4)+1);
    REQUIRE(cache.space_used() == strlen(val4)+1);
  }

  SECTION( "get should not retreive a key that was inserted and deleted" ) {
    cache.set("k1", val1, strlen(val1)+1);

    REQUIRE(strcmp(cache.get("k1", size), val1) == 0);   // string comparision
    REQUIRE(size == strlen(val1)+1);
    REQUIRE(cache.space_used() == 2);

    cache.del("k1");
    REQUIRE(cache.get("k1", size) == NULL);
    REQUIRE(size == 0);
    REQUIRE(cache.space_used() == 0);
  }

  SECTION( "resetting should return empty cache" ) {
    cache.set("k1", val1, strlen(val1)+1);
    cache.set("k2", val2, strlen(val2)+1);

    cache.reset();
    REQUIRE(cache.space_used() == 0);
    REQUIRE(cache.get("k1", size)== NULL);
    REQUIRE(cache.get("k2", size)== NULL);
  }

}
