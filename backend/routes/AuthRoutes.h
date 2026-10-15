#pragma once
#include <string>
#include <cstdlib>
#include "../include/httplib.h"
#include "../include/json.hpp"
#include "../utils/FileDB.h"
using json = nlohmann::json;

namespace AuthRoutes {

    const std::string USERS_PATH = "../data/users.json";
    
    // Simple helper to generate User ID
    inline std::string generateUserId() {
        return "U-" + std::to_string(1000 + (rand() % 9000));
    }

    inline void addCORS(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    inline void setup(httplib::Server& svr) {

        // POST /api/login
        svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body  = json::parse(req.body);
                std::string uname = body.value("username", "");
                std::string pw    = body.value("password", "");

                json users = FileDB::read(USERS_PATH);
                int idx    = FileDB::findIndex(users, "username", uname);

                if (idx < 0 || users[idx].value("password","") != pw) {
                    res.status = 401;
                    res.set_content(json{{"success",false},{"message","Invalid credentials"}}.dump(), "application/json");
                    return;
                }

                json user = users[idx];
                user.erase("password");
                res.set_content(json{{"success",true},{"user",user}}.dump(), "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content(json{{"success",false},{"message","Bad request"}}.dump(), "application/json");
            }
        });

        // POST /api/register
        svr.Post("/api/register", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body  = json::parse(req.body);
                std::string uname = body.value("username", "");
                std::string pw    = body.value("password", "");
                std::string email = body.value("email",    "");

                if (uname.empty() || pw.empty() || email.empty()) {
                    res.status = 400;
                    res.set_content(json{{"success",false},{"message","All fields required"}}.dump(), "application/json");
                    return;
                }

                json users = FileDB::read(USERS_PATH);
                if (FileDB::findIndex(users, "username", uname) >= 0) {
                    res.status = 409;
                    res.set_content(json{{"success",false},{"message","Username already taken"}}.dump(), "application/json");
                    return;
                }

                json newUser = {
                    {"user_id",  generateUserId()},
                    {"username", uname},
                    {"password", pw},
                    {"email",    email},
                    {"role",     "user"},
                    {"bookings", json::array()}
                };
                users.push_back(newUser);
                FileDB::write(USERS_PATH, users);

                newUser.erase("password");
                res.set_content(json{{"success",true},{"user",newUser}}.dump(), "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content(json{{"success",false},{"message","Bad request"}}.dump(), "application/json");
            }
        });
    }
}
