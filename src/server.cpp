#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include "server.hpp"
#include "utils.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace my_program_state {
std::size_t request_count() {
  static std::size_t count = 0;
  return ++count;
}

std::time_t now() { return std::time(0); }
} // namespace my_program_state

namespace Server {

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
  Server *server;
  http_connection(Server *s, tcp::socket socket)
      : socket_(std::move(socket)), server(s) {}

  // Initiate the asynchronous operations associated with the connection.
  void start() {
    read_request();
    check_deadline();
  }

private:
  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  request request_;
  response response_;

  // The timer for putting a deadline on connection processing.
  net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(10)};

  // Asynchronously receive a complete request message.
  void read_request() {
    auto self = shared_from_this();

    http::async_read(
        socket_, buffer_, request_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
          boost::ignore_unused(bytes_transferred);
          if (!ec)
            self->process_request();
        });
  }

  // Determine what needs to be done with the request message.
  void process_request() {
    response_.version(request_.version());
    response_.keep_alive(false);

    switch (request_.method()) {
    case http::verb::get:
    case http::verb::post:
      response_.result(http::status::ok);
      response_.set(http::field::server, "Beast");
      create_response();
      break;

    default:
      // We return responses indicating an error if
      // we do not recognize the request method.
      response_.result(http::status::bad_request);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body())
          << "Invalid request-method '" << std::string(request_.method_string())
          << "'";
      break;
    }

    write_response();
  }

  // Construct a response message based on the program state.
  void create_response() {
    auto path = std::string(request_.target());

    // Read the header
    //   http::read(request_, parser);
    //    std::cout << "HOST: " << parser.get()[http::field::host] << std::endl;
    //    std::cout << "HOST: " << parser.get()["host"] << std::endl;

    std::map<std::string, handler_signature>::iterator it =
        server->d_path_handlers.begin();

    // Iterate over the map using Iterator till end.
    while (it != server->d_path_handlers.end()) {
      // Accessing KEY from element pointed by it.
      std::string word = it->first;
      std::smatch sm_res;
      std::regex e(word);
      if (std::regex_match(path, sm_res, e)) {
        //    std::cout << "Match found :" << word << std::endl;
        it->second(server, &request_, &response_);
        return;
      }

      // Increment the Iterator to point to next entry
      it++;
    }

    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "File not found\r\n";

    /*
        if (request_.target() == "/count") {
          response_.set(http::field::content_type, "text/html");
          beast::ostream(response_.body())
              << "<html>\n"
              << "<head><title>Request count</title></head>\n"
              << "<body>\n"
              << "<h1>Request count</h1>\n"
              << "<p>There have been " << my_program_state::request_count()
              << " requests so far.</p>\n"
              << "</body>\n"
              << "</html>\n";
        } else if (request_.target() == "/time") {
          response_.set(http::field::content_type, "text/html");
          beast::ostream(response_.body())
              << "<html>\n"
              << "<head><title>Current time</title></head>\n"
              << "<body>\n"
              << "<h1>Current time</h1>\n"
              << "<p>The current time is " << my_program_state::now()
              << " seconds since the epoch.</p>\n"
              << "</body>\n"
              << "</html>\n";
        } else {
          response_.result(http::status::not_found);
          response_.set(http::field::content_type, "text/plain");
          beast::ostream(response_.body()) << "File not found\r\n";
        }
        */
  }

  // Asynchronously transmit the response message.
  void write_response() {
    auto self = shared_from_this();

    response_.set(http::field::content_length, response_.body().size());

    http::async_write(socket_, response_,
                      [self](beast::error_code ec, std::size_t) {
                        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                        self->deadline_.cancel();
                      });
  }

  // Check whether we have spent enough time on this connection.
  void check_deadline() {
    auto self = shared_from_this();

    deadline_.async_wait([self](beast::error_code ec) {
      if (!ec) {
        // Close socket to cancel any outstanding operation.
        self->socket_.close(ec);
      }
    });
  }
};

// "Loop" forever accepting new connections.
void Server::http_server(tcp::acceptor &acceptor, tcp::socket &socket) {
  acceptor.async_accept(socket, [&](beast::error_code ec) {
    if (!ec)
      std::make_shared<http_connection>(this, std::move(socket))->start();
    http_server(acceptor, socket);
  });
}

//------------------------------------------------------------------------------

void Server::Attach(std::string path, handler_signature func) {
  d_path_handlers.insert(std::make_pair(path, func));
}

/*
bool Server::ExecPathHandler(
    http::request<Body, http::basic_fields<Allocator>> &&req) {
  path = std::string(req.target());
  auto handler = d_path_handlers.find(path);
  if (handler != d_path_handlers.end()) {
    std::cout << "Target handler found" << std::endl;
    handler->second();
    return true;
  }
  return false; // ok
}
*/
void Server::Start(const unsigned short port) {
  try {
    std::cout << "Starting server on port " << port << std::endl;
    try {
      auto const address = net::ip::make_address("0.0.0.0");
      net::io_context ioc{1};
      tcp::acceptor acceptor{ioc, {address, port}};
      tcp::socket socket{ioc};
      http_server(acceptor, socket);
      ioc.run();
    } catch (std::exception const &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

} // namespace Server
