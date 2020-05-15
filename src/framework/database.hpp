//
// Created by Martin Drewes on 05/05/2020.
//

#ifndef WOLF_DATABASE_H
#define WOLF_DATABASE_H

#include <iostream>
#include <string>
#include <variant>
#include <any>
#include <sqlite3.h>
#include <vector>
#include <sstream>


class RecordNotFound : public std::exception {
    virtual const char *what() const throw() { return "RecordNotFound"; }
};


struct Field {
    char *name;
    char *type;
    int maxlength = 200;
};

class FieldValue {
public:
    std::variant<int, std::string> data;

    FieldValue(int x) { data = x; }

    FieldValue(char *x) { data = std::string(x); }

    FieldValue(std::string x) { data = x; }

    operator int() const {
        return std::get<int>(data);
    }

    operator std::string() const {
        return std::get<std::string>(data);
    }
};

template<typename T, const int... Is>
T make_obj_from_seq(const std::integer_sequence<int, Is...>, std::vector<FieldValue> data) {
    return T{data.at(Is)...};
}

template<typename T, int N>
T make_object(std::vector<FieldValue> data) {
    return make_obj_from_seq<T>(std::make_integer_sequence<int, N>{}, data);
}

using FieldVariant = std::variant<int, std::string>;
using RowVector = std::vector<FieldVariant>;

class Database {
public:
    sqlite3 *db;
    std::string filename_d;

    Database(std::string filename) {
        filename_d = filename;
        std::cout << "SQLite: " << sqlite3_libversion() << std::endl;
        int rc = sqlite3_open(filename.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        } else {
            std::cout << "Opened database successfully" << std::endl;
        }
    }

    ~Database() {
        sqlite3_close(db);
    };


    void Bind(sqlite3_stmt *stmt, std::initializer_list<std::any> list) {
        int r;
        int idx = 1;
        for (auto v: list) {
            //  std::cout << idx << " bind... " << v.type().name() << std::endl;

            if (auto ptr = std::any_cast<int>(&v)) {
                r = sqlite3_bind_int64(stmt, idx, *ptr);
                if (r != SQLITE_OK) {
                    std::cout << "bind error " << r << std::endl;
                }
                std::cout << idx << " bind-int " << *ptr << std::endl;
                idx++;
                continue;
            } else if (auto ptr = std::any_cast<std::string>(&v)) {
                const char *cc = (*ptr).c_str();
                r = sqlite3_bind_text(stmt, idx, cc, strlen(cc), SQLITE_TRANSIENT);
                if (r != SQLITE_OK) {
                    std::cout << "bind error " << r << std::endl;
                }
                std::cout << idx << " bind-string " << cc << " " << strlen(cc) << std::endl;
                idx++;
                continue;
            }
            std::cout << "bind error unknown type " << v.type().name() << std::endl;
        }
    }

    void Bind2(sqlite3_stmt *stmt, RowVector data) {
        int r;
        int idx = 1;
        std::cout << "Bind2 trying to bind " << data.size() << " values." << std::endl;

        for (auto v: data) {
            // std::cout << idx << std::endl;
            //   std::cout << " type " << (*v).type().name() << std::endl;

            try {
                if (std::holds_alternative<int>(v)) {
                    int *ptr = std::get_if<int>(&v);
//                    std::cout << "int" << std::endl;
                    r = sqlite3_bind_int64(stmt, idx, *ptr);
                    if (r != SQLITE_OK) {
                        std::cout << "Bind2 error (int) value index " << idx << std::endl;
                    }
                    std::cout << "Bind2 bound index " << idx << " to int " << std::endl;
                    idx++;
                    continue;
                }

                if (std::holds_alternative<std::string>(v)) {
                    std::string *ptr = std::get_if<std::string>(&v);
                    const char *cc = ptr->c_str();
                    r = sqlite3_bind_text(stmt, idx, cc, strlen(cc), SQLITE_TRANSIENT);
//                    std::cout << "string len=" << strlen(cc) << std::endl;
                    if (r != SQLITE_OK) {
                        std::cout << "Bind2 error (string) value index " << idx << std::endl;
                    }
                    std::cout << "Bind2 bound index " << idx << " to string " << std::endl;
                    idx++;
                    continue;
                }


            } catch (const std::exception &e) {
                std::cout << "Bind2 exception error: " << e.what() << std::endl;
            }

            std::cout << "bind error unknown type " << std::endl;
//            std::cout << "bind error unknown type " << (*v).type().name() << std::endl;
        }
    }

    void Execute(std::string sql, std::initializer_list<std::any> list = {}) {

        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }
        std::cout << "SQL : " << sql << std::endl;

        Bind(stmt, list);

        bool done = false;
        while (!done) {

            switch (sqlite3_step(stmt)) {

                case SQLITE_ROW:
                    std::cout << "row" << std::endl;
                    break;

                case SQLITE_DONE:
                    done = true;
                    std::cout << "done" << std::endl;
                    break;

                default:
                    done = true;
                    std::cout << "Execute error " << sql << std::endl;
            }
        }
        sqlite3_finalize(stmt);
    }


    void Execute2(std::string sql, RowVector data) {

        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }
        std::cout << "SQL : " << sql << std::endl;

        Bind2(stmt, data);

        bool done = false;
        while (!done) {

            switch (sqlite3_step(stmt)) {

                case SQLITE_ROW:
                    std::cout << "row" << std::endl;
                    break;

                case SQLITE_DONE:
                    done = true;
                    std::cout << "done" << std::endl;
                    break;

                default:
                    done = true;
                    std::cout << "Execute2 error " << sql << std::endl;
            }
        }
        sqlite3_finalize(stmt);
    }


    template<typename T, typename ST>
    void Insert2(T obj) {
        RowVector data;
        ST::ToDb(obj, data);
        std::string table = ST::TableName;

        std::cout << obj << std::endl;
        std::cout << ST::IndexName << std::endl;

        std::ostringstream sql;

        sql << "INSERT INTO " + table + " (";
        for (int i = 0; i < ST::Fields.size(); i++) {
            if (ST::Fields[i] != ST::IndexName) {
                sql << ST::Fields[i] << ",";
            }
        }
        sql.seekp(-1, sql.cur);
        sql << ") VALUES (";

        for (int i = 0; i < ST::Fields.size(); i++) {
            std::cout << ST::Fields[i] << std::endl;
            if (ST::Fields[i] != ST::IndexName) {
                sql << "?,";
            }
        }
        sql.seekp(-1, sql.cur);
        sql << ")";

        std::cout << sql.str() << std::endl;
        data.erase(data.begin());
        Execute2(sql.str(), data);


    }

    template<typename T, typename ST>
    void Update2(int id, T obj) {
        RowVector data;
        ST::ToDb(obj, data);
        std::string table = ST::TableName;
        std::cout << "LEN=" << data.size() << std::endl;

        for (auto x: data) {
            std::cout << x.index() << std::endl;
        }

        std::ostringstream sql;

        sql << "UPDATE " + table + " SET ";
        for (int i = 0; i < ST::Fields.size(); i++) {
            sql << ST::Fields[i] << "=?,";
        }
        sql.seekp(-1, sql.cur);
        sql << " WHERE id=?";

        std::cout << sql.str() << std::endl;
        // data.erase(data.begin());
        data.push_back(id); // the id we want to update
        Execute2(sql.str(), data);

    }


    template<typename T, typename ST>
    void Delete2(int id, T obj) {
        std::string table = ST::TableName;

        std::ostringstream sql;

        sql << "DELETE FROM " + table + " WHERE id=?";

        std::cout << sql.str() << std::endl;

        RowVector data;
        data.push_back(id); // the id we want to delete

        Execute2(sql.str(), data);
    }

    template<typename T, typename ST>
    std::vector<T> Select2(std::string sql = "", std::initializer_list<std::any> list = {}) {

        std::vector<T> result;
        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        std::stringstream ss;
        ss << "SELECT * FROM ";
        ss << ST::TableName;
        if (sql != "") {
            ss << " WHERE ";
            ss << sql;
        }
        std::cout << "SQL: " << ss.str() << std::endl;


        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }

        Bind(stmt, list);

        bool done = false;
        const unsigned char *txt;
        RowVector cl;
        while (!done) {
            auto step_res = sqlite3_step(stmt);
            if (step_res == SQLITE_ROW) {
                cl.clear();
                for (int i = 0; i < sqlite3_column_count(stmt); i++) {
                    int t = sqlite3_column_type(stmt, i);
                    if (t == 1) {
                        int r = sqlite3_column_int(stmt, i);
                        cl.push_back(r);
                    }
                    if (t == 3) {
                        auto r = sqlite3_column_text(stmt, i);
                        std::string rr = (char *) r;
                        cl.push_back(rr);
                    }
                }
                T t = T(); // make_object<T, T::Fields.size()>(cl);
                ST::FromDb(t, cl);
                result.push_back(t);
            } else if (step_res == SQLITE_DONE) {
                done = true;
                std::cout << "done" << std::endl;
            } else {
                done = true;
                std::cout << "err" << std::endl;
            }
        }
        sqlite3_finalize(stmt);
        return result;

    }

    template<typename T, typename ST>
    T GetById(int id) {
        auto rows = Select2<T, ST>("id=?", {id});
        if (rows.size() > 0) {
            return rows[0];
        }
        throw RecordNotFound();
    }


    template<typename T, int N>
    std::vector<T> Select(std::string sql, std::initializer_list<std::any> list = {}) {

        std::vector<T> result;
        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        std::cout << "SQL: " << sql << std::endl;

        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }

        Bind(stmt, list);

        bool done = false;
        const unsigned char *txt;
        std::vector<FieldValue> cl;
        while (!done) {
            auto step_res = sqlite3_step(stmt);
            if (step_res == SQLITE_ROW) {
                cl.clear();
                for (int i = 0; i < sqlite3_column_count(stmt); i++) {
                    int t = sqlite3_column_type(stmt, i);
                    if (t == 1) {
                        int r = sqlite3_column_int(stmt, i);
                        cl.push_back(FieldValue(r));
                    }
                    if (t == 3) {
                        auto r = sqlite3_column_text(stmt, i);
                        std::string rr = (char *) r;
                        cl.push_back(FieldValue(rr));
                    }
                }
                auto t = make_object<T, T::Fields.size()>(cl);
                result.push_back(t);
            } else if (step_res == SQLITE_DONE) {
                done = true;
                std::cout << "done" << std::endl;
            } else {
                done = true;
                std::cout << "err" << std::endl;
            }
        }
        sqlite3_finalize(stmt);
        return result;
    }

};

#endif //WOLF_DATABASE_H
