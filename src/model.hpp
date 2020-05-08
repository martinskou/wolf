//
// Created by Martin Drewes on 05/05/2020.
//

#ifndef WOLF_MODEL_H
#define WOLF_MODEL_H

#include <iostream>
#include <string>

#include "database.hpp"

namespace Model {

    extern Database *DB;

    void SetDB(Database *db);

/*
 * Table and TableFactory
 */


    class User {
    public:
        int id = 0;
        std::string username = "unknown";
        std::string password = "";
        std::string email = "";
        int type_id=0;

        User() {}
        User(int id_, std::string username_, std::string password_, std::string email_, int type_id_) :
        id(id_), username(username_), password(password_), email(email_), type_id(type_id_) {}

    };

    std::ostream &operator<<(std::ostream &os, const User &u);


    class Users {
    public:

        static constexpr const int TableColumns{4};
        static constexpr const char *TableName = "user";


        static void CreateTable(bool drop) {
            std::string sql;
            if (drop) {
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

        static User Get(int id) {
            std::string sql;
            sql = "SELECT * FROM 'user' WHERE id=?;";
            auto r=DB->Select<User,5>(sql, {id});

            return User();
        }


    };

}

#endif //WOLF_MODEL_H
