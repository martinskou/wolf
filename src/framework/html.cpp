//
// Created by Martin Drewes on 13/05/2020.
//

#include <any>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "html.hpp"


std::string html::node::render_children() {
    std::ostringstream os;
    for (auto c : children) {
        os << c->render() << std::endl;
    }
    return os.str();
}

void html::node::append(node *n) { children.push_back(n); }

html::node *html::node::operator<<(html::node *n) {
    this->append(n);
    return this;
}

std::string html::html::render() {
    std::ostringstream os;
    os << "<!DOCTYPE html>" << std::endl << "<html lang=\"da\">" << std::endl;
    os << render_children();
    os << "</html>" << std::endl;
    return os.str();
}

html::html *html::Html(std::initializer_list<html::node *> list) { return new html(list); }

html::head::head(std::string t, std::string d) {
    title = t;
    description = d;
}

std::string html::head::render() {
    std::ostringstream os;
    os << "<head>";
    os << "<title>" << title << "</title>" << std::endl;
    os << "<meta name=\"description\" content=\"" << description << "\"></meta>"
       << std::endl;
    os << node::render_children();
    os << "</head>";
    return os.str();
}

html::head *html::Head(std::string t, std::string d) { return new head(t, d); }

html::body::body(std::initializer_list<node *> list) : html::node(list) {}

std::string html::body::render() {
    std::ostringstream os;
    os << "<body>";
    os << node::render_children();
    os << "</body>";
    return os.str();
}

html::body *html::Body(std::initializer_list<html::node *> list) { return new body(list); }

std::string html::h1::render() {
    std::ostringstream os;
    os << "<h1 class=\"" << _cls << "\">" << text << "</h1>";
    return os.str();
}

html::h1 *html::H1(std::string t) { return new h1(t); }

std::string html::div::render() {
    std::ostringstream os;
    os << "<div>" << text << "</div>";
    return os.str();
}

html::div *html::Div(std::string t) { return new div(t); }

std::string html::field::render() {
    std::ostringstream os;
    os << "<div class='field'>";
    os << "<label class='label'>" << _caption
       << "</label><div class='control'>";
    if (_type == "text") {
        os << " <input class='input' type='" << _type << "' name='" << _name
           << "' id='' value='" << _value << "'><br>";
    } else if (_type == "static") {
        os << "" << _caption << " <input class='input' type='" << _type
           << "' readonly name='" << _name << "' id='' value='" << _value
           << "'><br>";
    } else if (_type == "textarea") {
        os << "" << _caption << " <textarea class='textarea' name='" << _name
           << "' id=''>" << _value << "</textarea><br>";
    } else {
        os << "" << _caption << " unknown type " << _type << "<br>";
    }
    os << "</div>";
    os << "</div>";
    return os.str();
}

html::field *html::Field(std::string t, std::string c, std::string n, std::string v) {
    return new field(t, c, n, v);
}

std::string html::select::render() {
    std::ostringstream os;
    os << "<div class='field'>";
    os << "<label class='label'>" << _caption
       << "</label><div class='control'>";

    os << "<select class='select' name='" << _name << "' >";
    for (auto o : _options) {
        if (utils::i2s(o.first) == _value) {
            os << "<option selected value='" << o.first << "'>" << o.second
               << "</option>";
        } else {
            os << "<option value='" << o.first << "'>" << o.second << "</option>";
        }
    }
    os << "</select>";
    os << "</div></div>";

    return os.str();
}

html::select *html::Select(std::string c, std::string n, std::string v, std::vector<std::pair<int, std::string>> o) {
    return new select(c, n, v, o);
}

std::string html::form::render() {
    std::ostringstream os;
    os << "<form method='post' action=''><h3>" << title << "</h3>";
    os << node::render_children();
    os << "</form>";
    return os.str();
}

html::form *html::Form(std::string t, std::initializer_list<html::node *> list) {
    return new form(t, list);
}

std::string html::button::render() {
    std::ostringstream os;
    os << "<button class='button' type='" << _type << "'>" << _caption
       << "</button>";
    return os.str();
}

html::button *html::Button(std::string t, std::string c) { return new button(t, c); }

std::string html::link(std::string title, std::string url, bool confirm, std::string cls) {
    std::ostringstream os;
    os << "<a href=\"" << url;
    os << "\"";
    if (confirm) {
        os << " onclick=\"return confirm('Bekræft " << boost::to_lower_copy(title)
           << "');\"";
    }
    os << " class='" << cls << "'";
    os << ">";
    os << title << "</a>";
    return os.str();
}
