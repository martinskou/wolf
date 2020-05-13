#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <thread>

#include "server.hpp"
#include "model.hpp"
#include "database.hpp"
#include "crud.hpp"

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




int main(int argc, char *argv[]) {
    auto db = Database("test.sqlite");
    Model::SetDB(&db);

    Model::UserData::CreateTable(false);

    /*
    Model::User u;
    if (Model::UserData::Get(u, 1)) {
        std::cout << u << std::endl;
    }

    Model::User x{2, "xxx", "y", "z", 1};
    Model::UserData::Insert2(x);
    */

    Server::Server s = Server::Server();
    s.Attach(Server::GET, "/demo/", handler_demo);
    s.Attach(Server::GET, "/de.*", handler_demo);
    s.Attach(Server::GET, "/dex", handler_demo);
    s.Attach(Server::GET, "^/bio/(.*)/$", handler_demo);

    auto c = Crud::Crud<Model::User, Model::UserSerializer>
            (Model::DB, "Users","/crud/user",
             [](std::string q) -> std::vector<Model::User> { return Model::DB->Select2<Model::User, Model::UserSerializer>(); },
             [](int id) -> Model::User { return Model::DB->GetById<Model::User, Model::UserSerializer>(id); },
             [](Model::User t) { Model::DB->Insert2<Model::User, Model::UserSerializer>(t); }, // insert
             [](Model::User t, int id) { Model::DB->Update2<Model::User, Model::UserSerializer>(id,t); }, // update
             [](Model::User t, int id) { Model::DB->Delete2<Model::User, Model::UserSerializer>(id,t); } // delete
            );

    c.AddField(Crud::Field<Model::User>(
            "Id", "id", "static", true, true,
            [](Model::User o) -> std::string { return utils::i2s(o.id); },
            [](Model::User &o, std::string &&v) { o.id = utils::s2i(v); }));

    c.AddField(Crud::Field<Model::User>(
            "Email", "email", "text", true, true,
            [](Model::User o) -> std::string { return o.email; },
            [](Model::User &o, std::string &&v) { o.email = v; }));

    c.AddField(Crud::Field<Model::User>(
            "Password", "password", "text", true, true,
            [](Model::User o) -> std::string { return o.password; },
            [](Model::User &o, std::string &&v) { o.password = v; }));


    c.AddAction(html::ListAction("New user", c.root+"/edit/0"));


    c.Attach(s);

    s.Start(2345);
}