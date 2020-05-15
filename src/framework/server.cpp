#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <tuple>

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

      std::string GET = "GET";
      std::string POST = "POST";


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

            auto lp = path.find_last_of('?');
            if (lp != std::string::npos) {
                path.erase(lp, std::string::npos);
            }

            handler_vector::iterator it =
                    server->d_path_handlers.begin();

            // Iterate over the map using Iterator till end.
            while (it != server->d_path_handlers.end()) {
                // Accessing KEY from element pointed by it.
                std::string method = std::get<0>(*it); // method
                if (method == request_.method_string()) {
                    std::string word = std::get<1>(*it); // path
                    std::smatch sm_res;
                    std::regex e(word);
                    if (std::regex_match(path, sm_res, e)) {
                        //    std::cout << "Match found :" << word << std::endl;
                        try {
                            auto hfunc=std::get<2>(*it);
                            std::string arg=std::get<3>(*it);
                            hfunc(server, &request_, &response_, arg);
                        } catch (std::exception const &e) {
                            std::cerr << "Unhandled error in handler: " << e.what() << std::endl;
                        }

                        return;
                    }
                }

                // Increment the Iterator to point to next entry
                it++;
            }

            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "File not found\r\n";
        }

        // Asynchronously transmit the response message.
        void write_response() {
            auto self = shared_from_this();

            response_.set(http::field::content_length, response_.body().size());
            response_.set(http::field::server, "Wolf");

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

    void Server::Attach(std::string method, std::string path, handler_signature func, std::string arg) {
        d_path_handlers.push_back(std::tuple(method, path, func, arg));
    }

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
                std::cerr << "Unhandled inner server error: " << e.what() << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Unhandled server error: " << e.what() << std::endl;
        }
    }

    void redirect(response *res, std::string url) {
        res->result(boost::beast::http::status::found);
        res->set(boost::beast::http::field::location, url);
//        auto out = boost::beast::ostream(res->body());
//        out << "<html><meta charset='utf-8'>\n"

    }


    void send_file(std::string fname, response *res) {

        auto out = boost::beast::ostream(res->body());

        auto ifs = std::make_shared<std::ifstream>();

        ifs->open(fname, std::ifstream::in | std::ios::binary | std::ios::ate);

        if (*ifs) {
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);

            res->set(boost::beast::http::field::content_length, to_string(length));


            if (utils::ends_with(fname, ".js")) {
                res->set(boost::beast::http::field::content_type, "application/javascript; charset=utf-8");
            }
            if (utils::ends_with(fname, ".svg")) {
                res->set(boost::beast::http::field::content_type, "image/svg+xml");
            }
            if (utils::ends_with(fname, ".png")) {
                res->set(boost::beast::http::field::content_type, "image/png");
            }
            if (utils::ends_with(fname, ".jpg")) {
                res->set(boost::beast::http::field::content_type, "image/jpg");
            }
            if (utils::ends_with(fname, ".css")) {
                res->set(boost::beast::http::field::content_type, "text/css");
            }

            static std::vector<char> buffer(131072); // Safe when server is running on one thread
            std::streamsize read_length;
            bool done = false;
            while (!done) {
                read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount();
                if (read_length) {
                    if (read_length != static_cast<std::streamsize>(buffer.size())) {
                        done=true;
                    }
                    out.write(&buffer[0],read_length);
                }
            }

        } else {
            std::cout << "file " << fname << " not found." << std::endl;
            res->set(boost::beast::http::field::content_type, "text/html");
            res->result(boost::beast::http::status::not_found);
            out << "404 not found";
        }

    }


    void handler_file(Server *s, request *req, response *res, std::string root_path) {
        auto form = Form(req);

        std::string path = form.path[2];
        std::string fname = root_path + path;

        std::cout << "FILE: " << path << " [" << fname << "]" << std::endl;

        send_file(fname,res);
    }


    void handler_single_file(Server *s, request *req,
                             response *res, std::string static_file) {
        auto form = Form(req);

        res->set(boost::beast::http::field::content_type, "text/html");
        auto out = boost::beast::ostream(res->body());

        send_file(static_file,res);
    }



} // namespace Server
