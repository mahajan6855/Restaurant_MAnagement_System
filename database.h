
#pragma once

#include <mysql.h>
#include <string>
#include <vector>
#include <map>

class Database {
private:
    MYSQL* connection;

public:
    Database();
    ~Database();

    bool connect();

    bool execute(const std::string& sql);

    std::vector<std::map<std::string, std::string>>
    query(const std::string& sql);

    std::string escape(const std::string& value);

    MYSQL* getConnection();
};
