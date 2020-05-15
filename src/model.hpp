//
// Created by Martin Drewes on 05/05/2020.
//

#ifndef WOLF_MODEL_H
#define WOLF_MODEL_H

#include <iostream>
#include <string>
#include <array>
#include <variant>

#include "framework/database.hpp"

namespace Model {

    extern Database *DB;

    void SetDB(Database *db);


    class User {
    public:
        int id = 0;
        std::string username = "unknown";
        std::string password = "";
        std::string email = "";
        int type_id = 0;
    };

    std::ostream &operator<<(std::ostream &os, const User &u);



    class UserSerializer {
    public:

        static constexpr const char *TableName = "user";
        static constexpr const char *IndexName = "id";

        static constexpr const std::array<const char *, 5> Fields{"id", "username", "password", "email", "type_id"};

        static void ToDb(const User &u, RowVector &values) {
            // convert user u to values in vector
            // these values can be saved in db.
            values.emplace_back(u.id);
            values.emplace_back(u.username);
            values.emplace_back(u.password);
            values.emplace_back(u.email);
            values.emplace_back(u.type_id);
            std::cout << "ToDb" << std::endl;
        }

        static void FromDb(User &u, const RowVector &values) {
            // convert values to user and return the user in u.
            try {
                u.id = std::get<int>(values.at(0));
                u.username = std::get<std::string>(values.at(1));
                u.password = std::get<std::string>(values.at(2));
                u.email = std::get<std::string>(values.at(3));
                u.type_id = std::get<int>(values.at(4));
            } catch (const std::exception &e) {
                std::cout << "FromDb Error: " << e.what() << std::endl;
            }
        }

    };

    class UserData {
    public:


        static void CreateTable(bool drop) {
            std::string sql;
            if (drop) {
                std::cout << "DROP TABLES!" << std::endl;
                sql = "DROP TABLE IF EXISTS 'user';";
                DB->Execute(sql);
                sql = "DROP TABLE IF EXISTS 'user_type';";
                DB->Execute(sql);
            }
            sql = "CREATE TABLE IF NOT EXISTS 'user_type' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL , 'name' TEXT NOT NULL );";
            DB->Execute(sql);
            sql = "CREATE TABLE IF NOT EXISTS 'user' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL , 'username' TEXT NOT NULL , 'password' TEXT NOT NULL, 'email' TEXT NOT NULL , 'type_id' INTEGER NOT NULL , FOREIGN KEY( type_id ) REFERENCES 'user_type' ( id ) );";
            DB->Execute(sql);
        }

        static bool Get(User &u, int id) {
            std::string sql;
            sql = "id=?;";
            auto r = DB->Select2<User, UserSerializer>(sql, {id});
            if (r.size() > 0) {
                u = r[0];
                return true;
            }
            return false;
        }

        static void Insert(User &u) {
            DB->Insert2<User, UserSerializer>(u);
        }


    };

}

#endif //WOLF_MODEL_H
