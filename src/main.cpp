#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <thread>

#include "server.hpp"
#include "model.hpp"
#include "database.hpp"

void handler_demo(Server::Server *s, Server::request *req,
                  Server::response *res) {

    auto form = Server::Form(req);
    auto session = Server::Session(req, res);

    auto path = std::string(req->target());

    res->set(boost::beast::http::field::content_type, "text/html");
    auto out = boost::beast::ostream(res->body());
    out << "<html><meta charset='utf-8'>\n"
        << "<head><title>T</title></head>\n"
        << "<body>\n"
        << "<h1>FROM DEMO</h1>\n"
        << "PATH: " << path << "<br>";

    for (auto const &field : *req)
        out << "FIELD: " << field.name() << " = " << field.value() << "<br>";

    out << "<form method='post' action=''>"
        << "<input type='text' name='name'>"
        << "<input type='submit' value='save'>"
        << "</form>"
           "x="
        << form["x"]
        << "<br>"
           "name="
        << form["name"] << "<br>"
        << "session: " << session.get_cookie("session") << "<br>"
        << "y:" << session.get_cookie("y") << "<br>"
        << "</body>\n"
        << "</html>\n";

    session.set_cookie("y", "14");

    //  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    //    std::cout << "Body: " << request_.body() << std::endl;
    /*
          std::cout << "------" << std::endl;
          std::cout << "DATA:   "
                    << boost::beast::buffers_to_string(request_.body().data())
                    << std::endl;
          std::cout << "------" << std::endl;

          std::cout << "REQUEST" << std::endl;
          std::cout << request_ << std::endl;
          std::cout << "------" << std::endl;
    */
    //    http::request_parser<http::dynamic_body> parser;

    //  std::cout << "Server: " << req->[http::field::host] << std::endl;
}

/*
void handler_set(app::App *a, app::State *s, HttpResponse response, HttpRequest
request) {

  std::ostringstream os;
  os << "your arg : " << request->path_match[1].str() << " ";
  os << a->name;

  s->cookies.Print();
  s->session->Print();

  s->session->data["x"] = "y";

  response->write(SimpleWeb::StatusCode::success_ok, os.str(),
s->cookies.GetHeader());
}
*/

/*
template<typename T, typename F>
T make_object() {
    std::cout << F::TableColumns << std::endl;
    auto is = std::make_integer_sequence<int, F::TableColumns>{};
    std::cout <<  is[0]  << std::endl;

    return T{};
}
*/


class Data {
public:
    std::any data;

    Data(int x) { data=std::any(x); }
    Data(char *x) { data=std::any(std::string(x)); }

    operator int() const
    {
        return std::any_cast<int>(data);
    }
    operator std::string() const
    {
        return std::any_cast<std::string>(data);
    }
};

template<typename T, const int... Is>
T make_obj_from_seq(const std::integer_sequence<int, Is...>, std::vector<Data> data)
{
    return T{data.at(Is)...};
}

template<typename T, int N>
T make_object(std::vector<Data> data)
{
    return make_obj_from_seq<T>(std::make_integer_sequence<int, N>{},data);
}


int main(int argc, char *argv[]) {
    auto db = Database("test.sqlite");
    Model::SetDB(&db);

    Model::Users::CreateTable(true);

    auto u=Model::Users::Get(1);
    std::cout << u << std::endl;

//    std::vector<Data> cl{13,"a","b","c"};

//    std::cout << "MAKE_OBJ" << std::endl;
//    std::cout << make_object<Model::User,4>(cl) << std::endl;

    Server::Server s = Server::Server();
    s.Attach("/demo/", handler_demo);
    s.Attach("/de.*", handler_demo);
    s.Attach("/dex", handler_demo);
    s.Attach("^/bio/(.*)/$", handler_demo);
    s.Start(2345);
}