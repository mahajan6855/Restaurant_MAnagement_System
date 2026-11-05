
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <filesystem>

#include "httplib.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

mutex fileMutex;

const string MENU_FILE = "data/menu.json";
const string ORDER_FILE = "data/orders.json";

json readFile(const string& filename) {
    lock_guard<mutex> lock(fileMutex);

    ifstream file(filename);

    if (!file.is_open())
        return json::array();

    json data;

    try {
        file >> data;
    } catch (...) {
        return json::array();
    }

    return data;
}

void writeFile(const string& filename, const json& data) {
    lock_guard<mutex> lock(fileMutex);

    ofstream file(filename);

    if (!file.is_open())
        throw runtime_error("Cannot write data file");

    file << data.dump(4);
}

int nextId(const json& items) {
    int id = 1;

    for (const auto& item : items) {
        if (item.contains("id"))
            id = max(id, item["id"].get<int>() + 1);
    }

    return id;
}

int main() {

    filesystem::create_directories("data");

    if (!filesystem::exists(MENU_FILE)) {
        json menu = json::array({
            {
                {"id", 1},
                {"name", "Margherita Pizza"},
                {"category", "Pizza"},
                {"price", 199},
                {"description", "Classic cheese pizza"}
            },
            {
                {"id", 2},
                {"name", "Veg Burger"},
                {"category", "Burger"},
                {"price", 99},
                {"description", "Fresh vegetable burger"}
            },
            {
                {"id", 3},
                {"name", "Paneer Biryani"},
                {"category", "Rice"},
                {"price", 179},
                {"description", "Delicious paneer biryani"}
            }
        });

        writeFile(MENU_FILE, menu);
    }

    if (!filesystem::exists(ORDER_FILE))
        writeFile(ORDER_FILE, json::array());

    httplib::Server server;

    server.set_mount_point("/", "./public");

    // GET FOOD MENU
    server.Get("/api/menu",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json menu = readFile(MENU_FILE);

            res.set_content(
                menu.dump(),
                "application/json"
            );
        }
    );

    // ADD FOOD ITEM
    server.Post("/api/menu",
        [](const httplib::Request& req,
           httplib::Response& res) {

            try {
                json item = json::parse(req.body);

                if (!item.contains("name") ||
                    !item.contains("price") ||
                    !item["name"].is_string() ||
                    !item["price"].is_number() ||
                    item["name"].get<string>().empty() ||
                    item["price"].get<double>() < 0) {

                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid food item"})",
                        "application/json"
                    );
                    return;
                }

                json menu = readFile(MENU_FILE);

                item["id"] = nextId(menu);

                if (!item.contains("category"))
                    item["category"] = "Other";

                if (!item.contains("description"))
                    item["description"] = "";

                menu.push_back(item);

                writeFile(MENU_FILE, menu);

                res.set_content(
                    item.dump(),
                    "application/json"
                );

            } catch (...) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid request"})",
                    "application/json"
                );
            }
        }
    );

    // DELETE FOOD ITEM
    server.Delete(R"(/api/menu/(\d+))",
        [](const httplib::Request& req,
           httplib::Response& res) {

            int id = stoi(req.matches[1]);

            json menu = readFile(MENU_FILE);

            auto oldSize = menu.size();

            json updated = json::array();

            for (auto& item : menu) {
                if (item["id"] != id)
                    updated.push_back(item);
            }

            if (updated.size() == oldSize) {
                res.status = 404;
                res.set_content(
                    R"({"error":"Food not found"})",
                    "application/json"
                );
                return;
            }

            writeFile(MENU_FILE, updated);

            res.set_content(
                R"({"message":"Food deleted successfully"})",
                "application/json"
            );
        }
    );

    // PLACE CUSTOMER ORDER
    server.Post("/api/orders",
        [](const httplib::Request& req,
           httplib::Response& res) {

            try {
                json request = json::parse(req.body);

                if (!request.contains("customer") ||
                    !request["customer"].is_string() ||
                    request["customer"].get<string>().empty() ||
                    !request.contains("items") ||
                    !request["items"].is_array() ||
                    request["items"].empty()) {

                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid order"})",
                        "application/json"
                    );
                    return;
                }

                json menu = readFile(MENU_FILE);
                json orders = readFile(ORDER_FILE);

                json orderItems = json::array();
                double total = 0;

                for (const auto& requested :
                     request["items"]) {

                    int foodId = requested.value("id", 0);
                    int quantity = requested.value("quantity", 0);

                    if (quantity < 1 || quantity > 100) {
                        res.status = 400;
                        res.set_content(
                            R"({"error":"Invalid quantity"})",
                            "application/json"
                        );
                        return;
                    }

                    bool found = false;

                    for (const auto& food : menu) {
                        if (food["id"] == foodId) {
                            double price =
                                food["price"].get<double>();

                            double subtotal = price * quantity;
                            total += subtotal;

                            orderItems.push_back({
                                {"id", foodId},
                                {"name", food["name"]},
                                {"price", price},
                                {"quantity", quantity},
                                {"subtotal", subtotal}
                            });

                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        res.status = 400;
                        res.set_content(
                            R"({"error":"Food item unavailable"})",
                            "application/json"
                        );
                        return;
                    }
                }

                json order = {
                    {"id", nextId(orders)},
                    {"customer", request["customer"]},
                    {"items", orderItems},
                    {"total", total},
                    {"status", "Pending"},
                    {"payment", "Unpaid"},
                    {"created", time(nullptr)}
                };

                orders.push_back(order);
                writeFile(ORDER_FILE, orders);

                res.status = 201;
                res.set_content(
                    order.dump(),
                    "application/json"
                );

            } catch (...) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid order request"})",
                    "application/json"
                );
            }
        }
    );

    // VIEW ALL ORDERS
    server.Get("/api/orders",
        [](const httplib::Request& req,
           httplib::Response& res) {

            json orders = readFile(ORDER_FILE);

            res.set_content(
                orders.dump(),
                "application/json"
            );
        }
    );

    // UPDATE ORDER STATUS
    server.Put(R"(/api/orders/(\d+)/status)",
        [](const httplib::Request& req,
           httplib::Response& res) {

            try {
                int id = stoi(req.matches[1]);
                json body = json::parse(req.body);

                string status = body.value(
                    "status", ""
                );

                if (status != "Pending" &&
                    status != "Preparing" &&
                    status != "Completed" &&
                    status != "Cancelled") {

                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid status"})",
                        "application/json"
                    );
                    return;
                }

                json orders = readFile(ORDER_FILE);
                bool found = false;

                for (auto& order : orders) {
                    if (order["id"] == id) {
                        order["status"] = status;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    res.status = 404;
                    res.set_content(
                        R"({"error":"Order not found"})",
                        "application/json"
                    );
                    return;
                }

                writeFile(ORDER_FILE, orders);

                res.set_content(
                    R"({"message":"Order updated"})",
                    "application/json"
                );

            } catch (...) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid request"})",
                    "application/json"
                );
            }
        }
    );

    cout << "Restaurant Management Server Started!\n";
    cout << "Open http://localhost:8080\n";

    server.listen("127.0.0.1", 8080);

    return 0;
}
