#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

namespace Server {

class Request {
public:
  std::string path;
};

class Server {
  std::map<std::string, std::function<void()>> d_path_handlers;
  friend class http_connection;
  void http_server(tcp::acceptor &acceptor, tcp::socket &socket);

public:
  void Start(const unsigned short port);
  void Attach(std::string path, std::function<void()> func);
  /*  bool
    ExecPathHandler(http::request<Body, http::basic_fields<Allocator>>
    &&req);*/
};

} // namespace Server