#pragma once
#include <string>
#include "../include/httplib.h"
#include "../include/json.hpp"
#include "../utils/FileDB.h"
#include "../models/Flight.h"
using json = nlohmann::json;

namespace AdminRoutes {

    const std::string FLIGHTS_PATH  = "../data/flights.json";
    const std::string BOOKINGS_PATH = "../data/bookings.json";
    const std::string USERS_PATH    = "../data/users.json";

    inline void addCORS(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    // Simple admin auth check (role field in body)
    inline bool isAdmin(const json& body) {
        return body.value("role","") == "admin";
    }

    inline void setup(httplib::Server& svr) {

        // GET /api/admin/bookings â€” all bookings
        svr.Get("/api/admin/bookings", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            json bookings = FileDB::read(BOOKINGS_PATH);
            json flights  = FileDB::read(FLIGHTS_PATH);

            // Enrich every booking with flight info
            for (auto& b : bookings) {
                int fidx = FileDB::findIndex(flights, "flight_id", b.value("flight_id",""));
                if (fidx >= 0) {
                    b["flight_info"] = {
                        {"airline",     flights[fidx].value("airline","MatAir")},
                        {"source",      flights[fidx].value("source","")},
                        {"destination", flights[fidx].value("destination","")}
                    };
                    for (const auto& s : flights[fidx]["schedules"]) {
                        if (s.value("schedule_id","") == b.value("schedule_id","")) {
                            b["flight_info"]["departure_time"] = s.value("departure_time","");
                            b["flight_info"]["arrival_time"]   = s.value("arrival_time","");
                            break;
                        }
                    }
                }
            }
            res.set_content(json{{"success",true},{"bookings",bookings}}.dump(),"application/json");
        });

        // GET /api/admin/users â€” all users (no passwords)
        svr.Get("/api/admin/users", [](const httplib::Request&, httplib::Response& res) {
            addCORS(res);
            json users = FileDB::read(USERS_PATH);
            for (auto& u : users) u.erase("password");
            res.set_content(json{{"success",true},{"users",users}}.dump(),"application/json");
        });

        // POST /api/admin/add-flight
        svr.Post("/api/admin/add-flight", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body = json::parse(req.body);

                json newFlight = {
                    {"flight_id",   body.value("flight_id",   "")},
                    {"airline",     "MatAir"},
                    {"source",      body.value("source",      "")},
                    {"destination", body.value("destination", "")},
                    {"schedules",   body.value("schedules",   json::array())}
                };

                if (newFlight["flight_id"].get<std::string>().empty()) {
                    res.status = 400;
                    res.set_content(json{{"success",false},{"message","flight_id required"}}.dump(),"application/json");
                    return;
                }

                json flights = FileDB::read(FLIGHTS_PATH);
                if (FileDB::findIndex(flights, "flight_id", newFlight["flight_id"].get<std::string>()) >= 0) {
                    res.status = 409;
                    res.set_content(json{{"success",false},{"message","Flight ID already exists"}}.dump(),"application/json");
                    return;
                }

                flights.push_back(newFlight);
                FileDB::write(FLIGHTS_PATH, flights);
                res.set_content(json{{"success",true},{"flight",newFlight}}.dump(),"application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",std::string(e.what())}}.dump(),"application/json");
            }
        });

        // POST /api/admin/update-flight
        svr.Post("/api/admin/update-flight", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body = json::parse(req.body);
                std::string fid = body.value("flight_id","");

                json flights = FileDB::read(FLIGHTS_PATH);
                int idx = FileDB::findIndex(flights, "flight_id", fid);
                if (idx < 0) {
                    res.status = 404;
                    res.set_content(json{{"success",false},{"message","Flight not found"}}.dump(),"application/json");
                    return;
                }

                // Merge updatable fields
                if (body.contains("source"))      flights[idx]["source"]      = body["source"];
                if (body.contains("destination")) flights[idx]["destination"] = body["destination"];
                if (body.contains("schedules"))   flights[idx]["schedules"]   = body["schedules"];

                FileDB::write(FLIGHTS_PATH, flights);
                res.set_content(json{{"success",true},{"flight",flights[idx]}}.dump(),"application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",std::string(e.what())}}.dump(),"application/json");
            }
        });

        // POST /api/admin/delete-flight
        svr.Post("/api/admin/delete-flight", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body = json::parse(req.body);
                std::string fid = body.value("flight_id","");
                json flights = FileDB::read(FLIGHTS_PATH);
                int idx = FileDB::findIndex(flights, "flight_id", fid);
                if (idx < 0) {
                    res.status = 404;
                    res.set_content(json{{"success",false},{"message","Flight not found"}}.dump(),"application/json");
                    return;
                }
                flights.erase(flights.begin() + idx);
                FileDB::write(FLIGHTS_PATH, flights);
                res.set_content(json{{"success",true},{"message","Flight deleted"}}.dump(),"application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",std::string(e.what())}}.dump(),"application/json");
            }
        });

        // GET /api/admin/stats
        svr.Get("/api/admin/stats", [](const httplib::Request&, httplib::Response& res) {
            addCORS(res);
            json bookings = FileDB::read(BOOKINGS_PATH);
            json flights  = FileDB::read(FLIGHTS_PATH);
            json users    = FileDB::read(USERS_PATH);

            int confirmed = 0, cancelled = 0;
            double revenue = 0;
            for (const auto& b : bookings) {
                if (b.value("status","") == "Confirmed") {
                    confirmed++;
                    revenue += b.value("total_price", 0.0);
                } else {
                    cancelled++;
                }
            }

            res.set_content(json{
                {"success",      true},
                {"total_flights",  (int)flights.size()},
                {"total_bookings", (int)bookings.size()},
                {"total_users",    (int)users.size()},
                {"confirmed",    confirmed},
                {"cancelled",    cancelled},
                {"revenue",      revenue}
            }.dump(), "application/json");
        });
    }
}
