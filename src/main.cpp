
#include "server.hpp"

void handler_demo() { std::cout << "demo..." << std::endl; }

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
  Server::Server s = Server::Server();
  s.Attach("/demo/", handler_demo);
  s.Attach("/de.*", handler_demo);
  s.Attach("/dex", handler_demo);
  s.Attach("^/bio/(.*)/$", handler_demo);
  s.Start(2345);
}