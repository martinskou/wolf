//
// Created by Martin Drewes on 05/05/2020.
//

#include "model.hpp"

namespace Model {

    Database *DB;

    void SetDB(Database *db) {
        DB = db;
    }

    std::ostream &operator<<(std::ostream &os, const User &u) {
        return os << "User<" << u.id << "/" << u.username << "/" << u.password << "/" << u.email << ">";
    }

}