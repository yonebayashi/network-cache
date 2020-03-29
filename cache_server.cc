#include "crow_all.h"
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define LOCALHOST_ADDRESS "127.0.0.1"
#define PORT 8080

using boost::asio::ip::tcp;
// Adapted from https://gist.github.com/spaghetti-/ca0c96237e7adef288d8cdc3a5ba5074


class Session : public std::enable_shared_from_this<Session>
{
private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];   // create a buffer of size max_length for data transfer

public:
  Session(boost::asio::io_service& ios)
    : socket_(ios)
  {
    std::cout << "Starting TCP connection\n";
  }
  ~Session()
  {
    std::cout << "Destroying TCP connection\n";
  }

  tcp::socket& get_socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, this,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

    socket_.async_write_some(
       boost::asio::buffer(data_, max_length),
       boost::bind(&Session::handle_write, this,
                 shared_from_this(),
                 boost::asio::placeholders::error,
                 boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(std::shared_ptr<Session>& s,
                   const boost::system::error_code& err,
                   size_t bytes_transferred)
  {
    if (!err) {
      std::cout << data_ << std::endl;
    } else {
      std::cerr << "err: " << err.message() << std::endl;
      socket_.close();
    }
  }

  void handle_write(std::shared_ptr<Session>& s,
                    const boost::system::error_code& err,
                    size_t bytes_transferred)
  {
    if (!err) {
      std::cout << "Server sent Hello message!" << std::endl;
    } else {
      std::cerr << "err: " << err.message() << std::endl;
      socket_.close();
    }
  }
};


class Server {
private:
  tcp::acceptor acceptor_;

  void start_accept() {
    std::shared_ptr<Session> session = std::make_shared<Session>(acceptor_.get_io_service());

    // asynchronous accept operation and wait for a new connection.
    acceptor_.async_accept(session->get_socket(),
                          boost::bind(&Server::handle_accept, this, session,
                          boost::asio::placeholders::error));
  }

public:
  //constructor for accepting connection from client
  Server(boost::asio::io_service& ios,
         short port) : acceptor_(ios, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }

  void handle_accept(std::shared_ptr<Session> session,
                     const boost::system::error_code& err)
  {
    if (!err) {
      session->start();
      start_accept();
    }
    else {
      std::cerr << "err: " + err.message() << std::endl;
      session.reset();
    }
  }
};


int main(int argc, char *argv[])
{
  // TODO: handle optional command line arguments

  // crow::SimpleApp app;

  try {
    boost::asio::io_service ios;
    Server server_(ios, PORT);
    ios.run();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  // app.stop();
  return 0;
}
