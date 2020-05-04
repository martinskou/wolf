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

#include "utils.hpp"

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

namespace Server {

class Server;

using request = http::request<http::dynamic_body>;
using response = http::response<http::dynamic_body>;

using handler_signature = std::function<void(Server *, request *, response *)>;

class Server {
  std::map<std::string, handler_signature> d_path_handlers;
  friend class http_connection;
  void http_server(tcp::acceptor &acceptor, tcp::socket &socket);

public:
  void Start(const unsigned short port);
  void Attach(std::string path, handler_signature func);
};

class Form {
  std::map<std::string, std::string> data;

public:
  Form(request *request_) {

    auto qs = std::string(request_->target());
    qs.erase(0, qs.find_first_of('?') + 1);
    // std::cout << qs << std::endl;

    data = utils::url_to_map(qs);

    auto cts = std::string((*request_)["Content-Type"]);
    //   std::cout << "Y> " << cts << std::endl;

    if (cts.size() > 0) {

      auto lp = cts.find_last_of(';');
      if (lp != std::string::npos) {
        cts.erase(cts.find_last_of(';') + 0, std::string::npos);
      }
      if (cts == "application/x-www-form-urlencoded") {
        auto c = boost::beast::buffers_to_string(request_->body().data());
        //        std::cout << c << std::endl;
        utils::url_to_map(&data, c);
      } else {
        std::cout << "error unknown content type " << cts << std::endl;
      }
    }
  }
  std::string operator[](std::string key) {
    auto i = data.find(key);
    if (i != data.end()) {
      return i->second;
    }
    return "";
  }
};

} // namespace Server