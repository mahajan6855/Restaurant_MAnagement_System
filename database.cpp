
#include "database.h"
#include "config.h"

#include <iostream>
#include <stdexcept>

Database::Database() {
    connection = mysql_init(nullptr);

    if (!connection)
        throw std::runtime_error("MySQL initialization failed");
}

Database::~Database() {
    if (connection)
        mysql_close(connection);
}

bool Database::connect() {

    if (!mysql_real_connect(
        connection,
        DB_HOST,
        DB_USER,
        DB_PASSWORD,
        DB_NAME,
        DB_PORT,
        nullptr,
        0
    )) {
        std::cerr << "MySQL Error: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    mysql_set_character_set(connection, "utf8mb4");

    std::cout << "Database Connected Successfully!\n";

    return true;
}

bool Database::execute(const std::string& sql) {

    if (mysql_real_query(
        connection,
        sql.c_str(),
        static_cast<unsigned long>(sql.length())
    ) != 0) {

        std::cerr << mysql_error(connection) << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result)
        mysql_free_result(result);

    return true;
}

std::vector<std::map<std::string, std::string>>
Database::query(const std::string& sql) {

    std::vector<std::map<std::string, std::string>> rows;

    if (mysql_real_query(
        connection,
        sql.c_str(),
        static_cast<unsigned long>(sql.length())
    ) != 0) {

        std::cerr << mysql_error(connection) << std::endl;
        return rows;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (!result)
        return rows;

    MYSQL_ROW row;

    int columns = mysql_num_fields(result);

    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    while ((row = mysql_fetch_row(result))) {

        std::map<std::string, std::string> record;

        for (int i = 0; i < columns; i++) {

            record[fields[i].name] =
                row[i] ? row[i] : "";
        }

        rows.push_back(record);
    }

    mysql_free_result(result);

    return rows;
}

std::string Database::escape(const std::string& value) {

    std::string escaped(value.size() * 2 + 1, '\0');

    unsigned long length = mysql_real_escape_string(
        connection,
        escaped.data(),
        value.c_str(),
        static_cast<unsigned long>(value.size())
    );

    escaped.resize(length);

    return escaped;
}

MYSQL* Database::getConnection() {
    return connection;
}
