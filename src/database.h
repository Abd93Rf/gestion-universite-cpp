#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using Row       = std::map<std::string, std::string>;
using ResultSet = std::vector<Row>;

class Database {
private:
    sqlite3*    db;
    std::string dbPath;

public:
    explicit Database(const std::string& dbPath = "student_management.db");
    ~Database();

    bool connect();
    void disconnect();
    bool isConnected() const;

    ResultSet   query(const std::string& sql);
    bool        execute(const std::string& sql);
    int         getLastInsertId();
    std::string escape(const std::string& value);

    // Initialise les tables et données de test au premier lancement
    void initSchema();
};

#endif // DATABASE_H