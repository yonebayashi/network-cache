# HW4: Let's network
**Authors: Hien Nguyen, David Tamas-Parris**

## How to Build

### Installation

First, make sure you have all the dependencies needed to run this project. We use **Boost v1.69.0** [download [here](https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz)], **Crow** (a single header file `crow_all.h` has been included in this repository, which can also be downloaded [here](https://github.com/ipkn/crow/releases/download/v0.1/crow_all.h)).

To install **Boost v1.69.0**, run the following commands

```
wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
tar -zxvf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh    # this will generate ./b2
sudo ./b2 install
```

You can check the installed version by running

```
cat /usr/local/include/boost/version.hpp | grep "BOOST_LIB_VERSION"
```
If you see something like below, you have the correct version installed.

```
#define BOOST_LIB_VERSION "1_69"
```

### Compiling and running

To compile all programs, run

```
make
```

Then start the server (default running on port `8080`),

```
./cache_server
```

`cache_server` accepts four optional command line arguments. `-m` maxmem, `-s` server, `-p` port, and `-t` threads. You can assume the server is always running on `127.0.0.1`, and ignore the thread count option for now. However, for the purpose of testing the cache, you can try setting _maxmem_ to some number (default is 8), and changing the default _port_.

To check the status of all networking tests, in another terminal tab, run

```
./test_cache_client --success
```

## Test Report

We tested the cache behavior over the network. The `cache_server` handles all cache operations as prompted by `cache_client` via TCP connection and HTTP requests, and then communicate back the results to `cache_client`.


Test name |  Description | Status
 --- | --- | ---
Get Nothing |  Assuring that `get` should not retrieve a key that wasn't inserted. | PASSED
Set/Get | `get` should retrieve an inserted key and modify `size` appropriately. | PASSED
Tracking Memory | Assuring that `space_used()` responds appropriately to new insertions. | PASSED
Modification | `get` should retrieve a key that was inserted and modified. | PASSED
Proper Insertion | Cache should stop accepting more keys when maxmem is exceeded | PASSED
Proper Deletion |  `get` should not retrieve a key that was inserted and deleted. | PASSED

Lastly, we ran `valgrind` to ensure no memory leaks on both the client and server side.
