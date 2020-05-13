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
#include <tuple>

#include "utils.hpp"

#ifndef SERVER_H
#define SERVER_H


using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

namespace Server {

    extern std::string GET;
    extern std::string POST;

    class Server;

    using request = http::request<http::dynamic_body>;
    using response = http::response<http::dynamic_body>;

    using handler_signature = std::function<void(Server *, request *, response *)>;
    using handler_vector = std::vector<std::tuple<std::string, std::string, handler_signature>>;


    class Server {
        handler_vector d_path_handlers;

        friend class http_connection;

        void http_server(tcp::acceptor &acceptor, tcp::socket &socket);

    public:
        void Start(const unsigned short port);

        void Attach(std::string method,std::string path, handler_signature func);
    };

    class Session {
    public:
        std::map<std::string, std::string> Cookies;
        response *res_d;

        Session(request *req, response *res) {
            res_d = res;

            // Print each cookie in the request
            std::cout << "Param list: " << std::endl;
            for (auto param : http::param_list((*req)[http::field::cookie]))
                std::cout << "Cookie '" << param.first << "' has value '" << param.second
                          << "'\n";

            auto cookie_str = std::string((*req)[http::field::cookie]);

            std::cout << "Session: " << cookie_str << std::endl;
            if (cookie_str != "") {

                Cookies = utils::double_split(cookie_str, ';', '=');

                /*
                      std::vector<std::string> cookie_vec;
                      boost::split(cookie_vec, cookie_str, [](char c) { return c == ';';
                   }); for (auto cs : cookie_vec) { std::vector<std::string>
                   one_cookie_str; boost::split(one_cookie_str, cs, [](char c) { return c
                   == '='; }); if (one_cookie_str.size() == 2) { auto k =
                   one_cookie_str[0]; auto v = one_cookie_str[1]; boost::trim(k);
                          boost::trim(v);
                          Cookies.insert(make_pair(k, v));
                        }
                      }
                */

            } else {
                auto u = utils::get_uuid();
                Cookies.insert(make_pair("session", u));
                //      res->set("Set-Cookie", "session=" + u);
            }
        }

        ~Session() {
            //  auto cs = std::string("");
            for (auto const &[key, val] : Cookies) {
                //  cs = cs + key + "=" + val + ",";
                res_d->insert("Set-Cookie", key + "=" + val);
                std::cout << "Set-Cookie " << key << " = " << val << std::endl;
            }
            //    std::cout << "Set-Cookie:" << cs << std::endl;
            //    res_d->(http::field::set_cookie, cs);
        }

        std::string get_cookie(std::string key) {
            auto r = Cookies.find(key);
            if (r == Cookies.end()) {
                return "";
            }
            return r->second;
        }

        void set_cookie(std::string key, std::string val) {
            Cookies.insert(make_pair(key, val));
        }
    };

    void redirect(response *res, std::string url);

    class Form {
        std::map<std::string, std::string> data;

    public:
        std::vector<std::string> path;

        Form(request *request_) {

            std::string qs = std::string(request_->target());

            std::string p = std::string(qs);
            auto lp = p.find_last_of('?');
            if (lp != std::string::npos) {
                p.erase(lp, std::string::npos);
            }
            path = utils::split(p, '/');

            qs.erase(0, qs.find_first_of('?') + 1);

            data = utils::url_to_map(qs);

            auto cts = std::string((*request_)["Content-Type"]);

            if (cts.size() > 0) {

                auto lp = cts.find_last_of(';');
                if (lp != std::string::npos) {
                    cts.erase(cts.find_last_of(';') + 0, std::string::npos);
                }
                if (cts == "application/x-www-form-urlencoded") {
                    auto c = boost::beast::buffers_to_string(request_->body().data());
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

#endif //SERVER_H
