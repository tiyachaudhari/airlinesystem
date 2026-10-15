#pragma once
#include <string>
#include "../include/httplib.h"
#include "../include/json.hpp"
#include "../utils/FileDB.h"
#include "../models/Flight.h"
using json = nlohmann::json;

namespace FlightRoutes {

    const std::string FLIGHTS_PATH = "../data/flights.json";

    inline void addCORS(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    inline void setup(httplib::Server& svr) {

        // GET /api/flights â€” return all flights
        svr.Get("/api/flights", [](const httplib::Request&, httplib::Response& res) {
            addCORS(res);
            try {
                json flights = FileDB::read(FLIGHTS_PATH);
                json result = json::array();
                for (auto& fj : flights) {
                    Flight f = Flight::fromJson(fj);
                    result.push_back(f.toJson());
                }
                res.set_content(json{{"success",true},{"flights",result}}.dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",e.what()}}.dump(), "application/json");
            }
        });

        // POST /api/search â€” search by source/destination/date
        svr.Post("/api/search", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body = json::parse(req.body);
                std::string src  = body.value("source",      "");
                std::string dst  = body.value("destination", "");
                std::string cls  = body.value("class",       "economy");

                json allFlights = FileDB::read(FLIGHTS_PATH);
                json matched    = json::array();

                for (auto& fj : allFlights) {
                    if (FileDB::iequal(fj.value("source",""), src) &&
                        FileDB::iequal(fj.value("destination",""), dst))
                    {
                        Flight f = Flight::fromJson(fj);
                        json fJson = f.toJson();
                        // Filter out schedules with no availability for chosen class
                        json filteredSchedules = json::array();
                        for (auto& sj : fJson["schedules"]) {
                            if (sj["available_seats"][cls].get<int>() > 0) {
                                filteredSchedules.push_back(sj);
                            }
                        }
                        if (!filteredSchedules.empty()) {
                            fJson["schedules"] = filteredSchedules;
                            matched.push_back(fJson);
                        }
                    }
                }

                res.set_content(
                    json{{"success",true},{"flights",matched},{"count",(int)matched.size()}}.dump(),
                    "application/json"
                );
            } catch (...) {
                res.status = 400;
                res.set_content(json{{"success",false},{"message","Bad request"}}.dump(), "application/json");
            }
        });

        // GET /api/flight/:id â€” single flight detail
        svr.Get(R"(/api/flight/([^/]+))", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            std::string fid = req.matches[1];
            json flights = FileDB::read(FLIGHTS_PATH);
            int idx = FileDB::findIndex(flights, "flight_id", fid);
            if (idx < 0) {
                res.status = 404;
                res.set_content(json{{"success",false},{"message","Flight not found"}}.dump(), "application/json");
                return;
            }
            Flight f = Flight::fromJson(flights[idx]);
            res.set_content(json{{"success",true},{"flight",f.toJson()}}.dump(), "application/json");
        });
    }
}
