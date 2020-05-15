//
// Created by Martin Drewes on 13/05/2020.
//

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

#include <boost/algorithm/string.hpp>


#include "template.hpp"
#include "utils.hpp"


namespace Templates {

    std::string Template::Render() {

        return text;
    }

    std::string Template::Render(tree t) {

        std::regex r("\\{\\{(.*?)\\}\\}");

        token_vector tokens = utils::tokenize(text, r);

        return text;
    }

    std::string Template::Render(std::map<std::string, std::string> t) {

        std::cout << text.size() << std::endl;
        std::string n=text;
        for (auto x: t) {
            n = n.replace(n.find("{{"+x.first+"}}"), x.first.length()+4, x.second);
        }
        n.shrink_to_fit();
        return n;
    }


    TemplateStore::TemplateStore(std::string path) {
        std::cout << "Load templates in " << path << std::endl;
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            auto fname = entry.path().filename().string();
            auto data = utils::read_file(entry.path().string());
            std::cout << entry.path().string() << " , " << fname << " " << data.size() <<  std::endl;
            templates.insert(
                    std::pair<std::string, Template>(fname, Template(data)));
        }
    }

    Template TemplateStore::Find(std::string name) {
        auto t = templates.find(name);
        if (t != templates.end()) {
            return t->second;
        }
        return Template("404 Template");
    }

    Template *TemplateStore::FindPtr(std::string name) {
        auto t = templates.find(name);
        if (t != templates.end()) {
            return &(t->second);
        }
        auto tf=Template("404 Template");
        return &tf;
    }

        TemplateStore *Store;

    void LoadTemplates(std::string path) {
        Store = new TemplateStore(path);
    }

    void write_json(std::basic_ostream<char> &o, tree t) {
        boost::property_tree::write_json(o, t);
    }

}