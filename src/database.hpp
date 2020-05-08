//
// Created by Martin Drewes on 05/05/2020.
//

#ifndef WOLF_DATABASE_H
#define WOLF_DATABASE_H

#include <iostream>
#include <string>
#include <any>
#include <sqlite3.h>
#include <vector>


class FieldValue {
public:
    std::any data;

    FieldValue(int x) { data=std::any(x); }
    FieldValue(char *x) { data=std::any(std::string(x)); }
    FieldValue(std::string x) { data=std::any(x); }

    operator int() const
    {
        return std::any_cast<int>(data);
    }
    operator std::string() const
    {
        return std::any_cast<std::string>(data);
    }
};

template<typename T, const int... Is>
T make_obj_from_seq(const std::integer_sequence<int, Is...>, std::vector<FieldValue> data)
{
    return T{data.at(Is)...};
}

template<typename T, int N>
T make_object(std::vector<FieldValue> data)
{
    return make_obj_from_seq<T>(std::make_integer_sequence<int, N>{},data);
}




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
        for (std::any v: list) {

            if (auto ptr = std::any_cast<int>(&v)) {
                r = sqlite3_bind_int64(stmt, idx, *ptr);
                if (r != SQLITE_OK) {
                    std::cout << "bind error " << r << std::endl;
                }
                idx++;
            }
            if (auto ptr = std::any_cast<std::string>(&v)) {
                r = sqlite3_bind_text(stmt, idx, (*ptr).c_str(), -1, 0);
                if (r != SQLITE_OK) {
                    std::cout << "bind error " << r << std::endl;
                }
                idx++;
            }
        }
    }

    void Execute(std::string sql) {

        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }

        bool done = false;
        while (!done) {

            switch (sqlite3_step(stmt)) {

                case SQLITE_ROW:
                    std::cout << "row" << std::endl;
                    //bytes = sqlite3_column_bytes(stmt, 0);
                    //text  = sqlite3_column_text(stmt, 1);
                    //printf ("count %d: %s (%d bytes)\n", row, text, bytes);
                    //row++;
                    break;

                case SQLITE_DONE:
                    done = true;
                    break;

                default:
                    done = true;
                    std::cout << "err" << std::endl;
            }
        }
        sqlite3_finalize(stmt);

    }



    template<typename T, int N>
    std::vector<T> Select(std::string sql, std::initializer_list<std::any> list) {

        std::vector<T> result;
        if (sqlite3_get_autocommit(db) == 0) {
            std::cout << "Autocommit disabled" << std::endl;
        }

        sqlite3_stmt *stmt;
        int r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            std::cout << "prepare error " << r << std::endl;
        }

        Bind(stmt, list);

        std::cout << "count " << sqlite3_column_count(stmt) << std::endl;

        bool done = false;
        const unsigned char *txt;
        std::vector<FieldValue> cl;
        while (!done) {

            auto step_res=sqlite3_step(stmt);

            if (step_res==SQLITE_ROW) {
                std::cout << "dcount " << sqlite3_data_count(stmt) << std::endl;
                std::cout << "row" << std::endl;
                //bytes = sqlite3_column_bytes(stmt, 0);
                txt = sqlite3_column_text(stmt, 1);
                std::cout << txt << std::endl;
                std::cout << sqlite3_column_origin_name(stmt, 1) << std::endl;
                //printf ("count %d: %s (%d bytes)\n", row, text, bytes);
                //row++;
                std::cout << "---" << std::endl;
                //std::initializer_list<std::any> cl;
                cl.clear();
                for (int i = 0; i < sqlite3_column_count(stmt); i++) {
                    int t=sqlite3_column_type(stmt, i);
                    std::cout << sqlite3_column_origin_name(stmt, i) << " / " <<
                              t << std::endl;

                    if (t==1) {
                        int r=sqlite3_column_int(stmt, i);
                        cl.push_back(FieldValue(r));
                        std::cout << "-int-" << std::endl;
                    }
                    if (t==3) {
                        auto r=sqlite3_column_text(stmt, i);
                        std::string rr=(char*)r;
                        cl.push_back(FieldValue(rr));
                        std::cout << "-string-" << std::endl;
                    }
                }
                std::cout << cl.size() << std::endl;
                std::cout << "-=-" << std::endl;
                auto t=make_object<T,N>(cl);
                result.push_back(t);

            }

           else  if (step_res==SQLITE_DONE) { done=true; }
            else { done=true;
                std::cout << "err" << std::endl;

            }

        }
        sqlite3_finalize(stmt);

        return result;
    }

};

#endif //WOLF_DATABASE_H
