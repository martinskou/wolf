//
// Created by Martin Drewes on 12/05/2020.
//

#ifndef MAIN_CRUD_H
#define MAIN_CRUD_H

#include <functional>

#include "server.hpp"
#include "database.hpp"
#include "html.hpp"
#include "template.hpp"


namespace Crud {


    template<class T, class TS>
    std::function<std::vector<T>(std::string)> GenSelect(Database *db) {
        return
                [db](std::string q) -> std::vector<T> {
                    return db->Select2<T, TS>();
                };
    }


    template<class T, class TS>
    std::function<T(int)> GenValue(Database *db) {
        return
                [db](int id) -> T {
                    return db->GetById<T, TS>(id);
                };
    }


    template<class T, class TS>
    std::function<void(T)> GenInsert(Database *db) {
        return
                [db](T t) {
                    return db->Insert2<T, TS>(t);
                };
    }


    template<class T, class TS>
    std::function<void(T, int)> GenUpdate(Database *db) {
        return
                [db](T t, int id) {
                    return db->Update2<T, TS>(id, t);
                };
    }


    template<class T, class TS>
    std::function<void(T, int)> GenDelete(Database *db) {
        return
                [db](T t, int id) {
                    return db->Delete2<T, TS>(id, t);
                };
    }


    template<class T, class TS>
    std::function<void(Server::request *req, Server::response *res, std::string ct, std::string where)>
    GenResponse(Templates::Template *tp) {
        return
                [tp](Server::request *req, Server::response *res, std::string ct, std::string where) {
                    auto x = tp->Render({{"content", ct}});
                    // std::string x = "x";
                    res->set(boost::beast::http::field::content_type, "text/html");
                    auto out = boost::beast::ostream(res->body());
                    out << x;
                };
    }


    template<class T>
    class Field {
    public:
        std::string title;
        std::string name;
        std::string type;
        bool show_list;
        bool show_edit;
        std::function<std::string(T)> val_func;
        std::function<void(T &, std::string)> set_func;
        std::function<std::vector<std::pair<int, std::string>>()>
                options_func;
        bool use_options{false};

        Field(std::string tit, std::string n, std::string t, bool sl, bool se, std::function<std::string(T)> vf,
              std::function<void(T &, std::string)> sf)
                : title(tit), name(n), type(t), show_list(sl), show_edit(se), val_func(vf), set_func(sf) {}

        void SetOptions(std::function<std::vector<std::pair<int, std::string>>()> o) {
            options_func = o;
            use_options = true;
        }
    };


    template<class T, class TS>
    class Crud {
        Database *DB;
        std::vector<Field<T>> fields;
        std::vector<html::ListAction> actions;
        std::vector<std::function<std::string(T)>> listactions;
        std::string searchfield;
        std::function<std::vector<T>(std::string)> vals_func;
        std::function<T(int)> val_func;
        std::function<void(T)> insert_func;
        std::function<void(T, int)> update_func;
        std::function<void(T, int)> delete_func;
        std::function<void(Server::request *req, Server::response *res, std::string, std::string)> render_func;


    public:
        std::string root;
        std::string name;

        Crud(Database *db, std::string n, std::string r, std::string sf,
             std::function<std::vector<T>(std::string)> vs, std::function<T(int)> v, std::function<void(T)> _if,
             std::function<void(T, int)> _uf, std::function<void(T, int)> _df,
             std::function<void(Server::request *req, Server::response *res, std::string, std::string)> _rf)
                : DB(db), name(n), root(r), searchfield(sf), vals_func(vs), val_func(v), insert_func(_if),
                  update_func(_uf), delete_func(_df), render_func(_rf) {}

        void ListView(Server::Server *s, Server::request *req,
                      Server::response *res) const {

            auto form = Server::Form(req);
            auto session = Server::Session(req, res);
            auto path = std::string(req->target());

            std::vector<T> data;
            auto sw = form["filter"];
            if (sw.size() > 0) {
                data = DB->Select2<T, TS>(searchfield + " LIKE ?", {sw + "%"});
            } else {
                data = DB->Select2<T, TS>();
            }
            std::cout << "LEN " << data.size() << std::endl;

            auto lcx = std::vector<html::ListColumn<T>>();
            for (auto const &f : fields) {
                if (f.show_list) {
                    lcx.push_back(html::ListColumn<T>(f.name, f.val_func));
                }
            }

            lcx.push_back(html::ListColumn<T>("Actions", [this](T t) -> std::string {
                std::ostringstream os;
                std::string sep = " / ";
                os << html::link("Slet", root + "/delete/" + std::to_string(t.id), true) << sep;
                os << html::link("Rediger", root + "/edit/" + std::to_string(t.id)) << sep;
                if (listactions.size() > 0) {
                    for (auto &la : listactions) {
                        os << la(t);
                        if (&la != &listactions.back()) {
                            os << sep;
                        }
                    }
                }
                return os.str();
            }));

            auto lx = html::List<T>(data, name, lcx, actions);
            render_func(req, res, lx.render(), "listview");
        }

        void EditView(Server::Server *s, Server::request *req,
                      Server::response *res) const {

            auto form = Server::Form(req);
            auto session = Server::Session(req, res);
            auto path = std::string(req->target());

            auto data = DB->Select2<T, TS>();

            auto id = utils::s2i(form.path[4]);
            std::cout << "edit id : " << id << std::endl;

            T c;
            try {
                c = val_func(id);
                std::cout << "found" << std::endl;
            } catch (const std::exception &ex) {
                std::cout << ex.what() << std::endl;
                c.id = 0;
            }

            auto frm = html::Form(name, {});
            for (auto const &f : fields) {
                if (f.show_edit) {
                    if (f.use_options) {
                        frm->AddField(html::Select(f.title, f.name, f.val_func(c), f.options_func()));
                    } else {
                        frm->AddField(html::Field(f.type, f.title, f.name, f.val_func(c)));
                    }
                }
            }
            frm->AddField(html::Button("submit", "Save"));
            render_func(req, res, frm->render(), "editview");
        }

        void EditSaveView(Server::Server *s, Server::request *req,
                          Server::response *res) const {

            auto form = Server::Form(req);
            auto id = utils::s2i(form.path[4]);
            std::cout << "edit id : " << id << std::endl;

            T c;

            if (id > 0) {
                try {
                    c = val_func(id);
                } catch (const std::exception &ex) {
                    std::cout << ex.what() << std::endl;
                    Server::redirect(res, root);
                    return;
                }
            }

            for (auto const &f : fields) {
                if (f.show_edit) {
                    f.set_func(c, form[f.name]);
                }
            }

            try {
                if (id > 0) {
                    update_func(c, id);
                } else {
                    insert_func(c);
                }
            } catch (const std::exception &ex) {
                std::cout << ex.what() << std::endl;
            }

            Server::redirect(res, root);
        }


        void DeleteView(Server::Server *s, Server::request *req,
                        Server::response *res) const {

            auto form = Server::Form(req);
            auto id = utils::s2i(form.path[4]);
            std::cout << "delete id : " << id << std::endl;

            T c;

            if (id > 0) {
                try {
                    c = val_func(id);
                } catch (const std::exception &ex) {
                    std::cout << ex.what() << std::endl;
                    Server::redirect(res, root);
                    return;
                }
            }

            try {
                delete_func(c, id);
            } catch (const std::exception &ex) {
                std::cout << ex.what() << std::endl;
            }

            Server::redirect(res, root);
        }


        void Attach(Server::Server &server) {

            server.Attach(Server::GET, root + "$", [this](Server::Server *s, Server::request *req,
                                                          Server::response *res, std::string arg) {
                this->ListView(s, req, res);
            });

            server.Attach(Server::GET, root + "/edit/(.*)$", [this](Server::Server *s, Server::request *req,
                                                                    Server::response *res,
                                                                    std::string arg) { this->EditView(s, req, res); });

            server.Attach(Server::POST, root + "/edit/(.*)$", [this](Server::Server *s, Server::request *req,
                                                                     Server::response *res, std::string arg) {
                this->EditSaveView(s, req, res);
            });

            server.Attach(Server::GET, root + "/delete/(.*)$", [this](Server::Server *s, Server::request *req,
                                                                      Server::response *res, std::string arg) {
                this->DeleteView(s, req, res);
            });


        }

        void AddField(Field<T> f) { fields.push_back(f); }

        void AddAction(html::ListAction a) { actions.push_back(a); }

    };
}

#endif //MAIN_CRUD_H
