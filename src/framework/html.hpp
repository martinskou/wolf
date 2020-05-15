#ifndef HTML_H
#define HTML_H


#include <any>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "utils.hpp"
#include <boost/algorithm/string.hpp>

namespace html {

    class node {
    public:
        std::string _cls;
        std::vector<node *> children;

        node() {}

        node(std::initializer_list<node *> list) : children(list) {}

        node *cls(std::string c) {
            _cls = c;
            return this;
        }

        virtual ~node() {
            for (auto c : children) {
                delete c;
            }
        };

        virtual std::string render() = 0;

        virtual std::string render_children();

        void append(node *n);

        node *operator<<(node *n);
    };

    class html : public node {
    public:
        html(std::initializer_list<node *> list) : node(list) {}

        virtual std::string render() override;
    };

    html *Html(std::initializer_list<node *> list);

    class head : public node {
    public:
        std::string title;
        std::string description;

        head(std::string t, std::string d);

        virtual std::string render() override;
    };

    head *Head(std::string t, std::string d);

    class body : public node {
    public:
        body(std::initializer_list<node *> list);

        std::string render();
    };

    body *Body(std::initializer_list<node *> list);

    class h1 : public node {
    public:
        std::string text;

        h1(std::string t) : text(t) {}

        std::string render();
    };

    h1 *H1(std::string t);

    class div : public node {
    public:
        std::string text;

        div(std::string t) : text(t) {}

        std::string render();
    };

    div *Div(std::string t);

    class field : public node {
    public:
        std::string _type;
        std::string _caption;
        std::string _name;
        std::string _value;

        field(std::string type, std::string caption, std::string name,
              std::string val)
                : _type(type), _caption(caption), _name(name), _value(val) {}

        std::string render();
    };

    field *Field(std::string t, std::string c, std::string n, std::string v);

    class select : public node {
    public:
        std::string _caption;
        std::string _name;
        std::string _value;
        std::vector<std::pair<int, std::string>> _options;

        select(std::string caption, std::string name, std::string val,
               std::vector<std::pair<int, std::string>> options)
                : _caption(caption), _name(name), _value(val), _options(options) {}

        std::string render();
    };

    select *Select(std::string c, std::string n, std::string v,
                   std::vector<std::pair<int, std::string>> o);

    class form : public node {
    public:
        std::string title;

        form(std::string t, std::initializer_list<node *> list)
                : node(list), title(t) {}

        std::string render();

        void AddField(node *f) { children.push_back(f); }
    };

    form *Form(std::string t, std::initializer_list<node *> list);

    class button : public node {
    public:
        std::string _type;
        std::string _caption;

        button(std::string type, std::string caption)
                : _type(type), _caption(caption) {}

        std::string render();
    };

    button *Button(std::string t, std::string c);

    std::string link(std::string title, std::string url, bool confirm = false,
                     std::string cls = "");

    class ListAction {
    public:
        std::string title;
        std::string url;

        ListAction(std::string t, std::string u) : title(t), url(u) {}
    };

    template<class T>
    class ListColumn {
    public:
        std::string title;
        std::function<std::string(T)> content;

        ListColumn(std::string t, std::function<std::string(T)> cl) : title(t), content(cl) {}
    };

    template<class T>
    class List : public node {
    public:
        std::vector<T> items;
        std::string title;
        std::vector<ListColumn<T>> columns;
        std::vector<ListAction> actions;

        List(std::vector<T> _items, std::string _title, std::vector<ListColumn<T>> _cols,
             std::vector<ListAction> _actions);

        std::string render() {
            std::ostringstream os;
            os << "<h1>" << title << "</h1>";

            os << "<input class='input' name='filter' "
                  "id='filter'><script>setup_filter()</script>";

            os << "<table class='table is-bordered'>";
            os << "<thead>";
            os << "<tr>";
            for (auto c : columns) {
                os << "<th>" << c.title << "</th>";
            }
            os << "</tr>";
            os << "</thead>";

            os << "<tbody>";
            for (auto i : items) {
                os << "<tr>";
                for (auto c : columns) {
                    os << "<td>" << c.content(i) << "</td>";
                }
                os << "</tr>";
            }
            os << "</tbody>";
            os << "</table>";

            for (auto a : actions) {
                os << "" << link(a.title, a.url, false, "button") << "";
            }

            return os.str();
        }
    };

    template<class T>
    List<T>::List(std::vector<T> _items, std::string _title, std::vector<ListColumn<T>> _cols,
                  std::vector<ListAction> _actions) {
        items = _items;
        title = _title;
        columns = _cols;
        actions = _actions;
    }

} // namespace html


#endif
