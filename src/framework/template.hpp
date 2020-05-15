//
// Created by Martin Drewes on 13/05/2020.
//

#ifndef MAIN_TEMPLATE_HPP
#define MAIN_TEMPLATE_HPP


#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Templates {

    using tree = boost::property_tree::ptree;
    using token_item = std::tuple<bool, std::string>;
    using token_vector = std::vector<token_item>;

    class MustacheSection {
    public:
        std::string name;
        std::vector<MustacheSection*> children;

        MustacheSection(std::string name_) {
            name=name_;
        }
    };


    class Mustache {
    public:
//        std::vector<std::variant<MustacheTxt, MustacheCmd>> items;
        MustacheSection root;

        Mustache(token_vector::iterator it, token_vector *tokens) : root("") {

            while (it!=tokens->end()) {
                token_item t=*it;
                if(std::get<0>(t)) {
                    std::cout << "T " << std::get<1>(t) << std::endl;
                } else {
                    std::cout << "Z " << std::get<1>(t) << std::endl;
                }
                it++;
            }


        }
    };

    class Template {
    public:
        std::string text;

         std::string Render();
         std::string Render(tree t);
         std::string Render(std::map<std::string, std::string> t);

        Template(std::string s) : text(s) {}
    };


    class TemplateStore {
    public:
        std::string path;
        std::map<std::string, Template> templates;

        TemplateStore(std::string path);
        Template Find(std::string name);
        Template* FindPtr(std::string name);
    };

    extern TemplateStore *Store;

    void LoadTemplates(std::string path);

    void write_json(std::basic_ostream<char> &o, tree t);

}

#endif //MAIN_TEMPLATE_HPP
