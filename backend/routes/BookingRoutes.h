#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include "../include/httplib.h"
#include "../include/json.hpp"
#include "../utils/FileDB.h"
#include "../models/Booking.h"
#include "../models/Flight.h"
using json = nlohmann::json;

namespace BookingRoutes {

    const std::string BOOKINGS_PATH = "../data/bookings.json";
    const std::string FLIGHTS_PATH  = "../data/flights.json";
    const std::string USERS_PATH    = "../data/users.json";
    
    // Simple helper to generate Booking ID (sequential)
    inline std::string generateBookingId(int currentCount) {
        return std::to_string(currentCount + 1);
    }
    
    // Simple helper for readable timestamp
    inline std::string getCurrentTime() {
        std::time_t now = std::time(0);
        char buf[80];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buf);
    }

    inline void addCORS(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    inline void setup(httplib::Server& svr) {

        // POST /api/book
        svr.Post("/api/book", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body = json::parse(req.body);
                std::string user_id     = body.value("user_id",     "");
                std::string flight_id   = body.value("flight_id",   "");
                std::string schedule_id = body.value("schedule_id", "");
                std::string cls         = body.value("class",       "economy");
                std::string travel_date = body.value("travel_date", "");
                json seat_arr           = body.value("seats",       json::array());
                json pax_arr            = body.value("passengers",  json::array());

                if (user_id.empty() || flight_id.empty() || schedule_id.empty()) {
                    res.status = 400;
                    res.set_content(json{{"success",false},{"message","Missing required fields"}}.dump(),"application/json");
                    return;
                }

                // Load flights
                json flights = FileDB::read(FLIGHTS_PATH);
                int fidx = FileDB::findIndex(flights, "flight_id", flight_id);
                if (fidx < 0) {
                    res.status = 404;
                    res.set_content(json{{"success",false},{"message","Flight not found"}}.dump(),"application/json");
                    return;
                }

                // Find schedule
                json& schedules = flights[fidx]["schedules"];
                int sidx = -1;
                for (int i = 0; i < (int)schedules.size(); ++i) {
                    if (schedules[i].value("schedule_id","") == schedule_id) { sidx = i; break; }
                }
                if (sidx < 0) {
                    res.status = 404;
                    res.set_content(json{{"success",false},{"message","Schedule not found"}}.dump(),"application/json");
                    return;
                }

                json& sched   = schedules[sidx];
                json& seatCls = sched["seats"][cls];

                // Validate seat availability
                std::vector<std::string> requestedSeats = seat_arr.get<std::vector<std::string>>();
                std::vector<std::string> bookedSeats    = seatCls["booked"].get<std::vector<std::string>>();
                int available = seatCls["total"].get<int>() - (int)bookedSeats.size();

                if ((int)requestedSeats.size() > available) {
                    res.status = 409;
                    res.set_content(json{{"success",false},{"message","Not enough seats available"}}.dump(),"application/json");
                    return;
                }

                // Check seat conflicts
                for (const auto& reqSeat : requestedSeats) {
                    for (const auto& bkSeat : bookedSeats) {
                        if (reqSeat == bkSeat) {
                            res.status = 409;
                            res.set_content(json{{"success",false},{"message","Seat "+reqSeat+" already booked"}}.dump(),"application/json");
                            return;
                        }
                    }
                }

                // Build passengers
                std::vector<Passenger> passengers;
                for (const auto& p : pax_arr) {
                    passengers.push_back(Passenger::fromJson(p));
                }

                // Save booking
                json bookings    = FileDB::read(BOOKINGS_PATH);
                
                // Generate simple sequential booking ID
                std::string bid = generateBookingId((int)bookings.size());
                
                auto booking    = makeBooking(cls, bid, user_id, flight_id, schedule_id,
                                              travel_date, getCurrentTime(), requestedSeats, passengers);

                // Calculate price with dynamic pricing
                Flight f = Flight::fromJson(flights[fidx]);
                double dynPrice = f.schedules[sidx].dynamicPrice(cls);
                booking->calculatePrice(dynPrice);

                // Mark seats as booked in flights.json
                for (const auto& seat : requestedSeats) {
                    seatCls["booked"].push_back(seat);
                }
                FileDB::write(FLIGHTS_PATH, flights);

                json bookingJson = booking->toJson();
                bookings.push_back(bookingJson);
                FileDB::write(BOOKINGS_PATH, bookings);


                // Update user's booking list
                json users = FileDB::read(USERS_PATH);
                int uidx = FileDB::findIndex(users, "user_id", user_id);
                if (uidx >= 0) {
                    users[uidx]["bookings"].push_back(bid);
                    FileDB::write(USERS_PATH, users);
                }

                res.set_content(json{{"success",true},{"booking",bookingJson}}.dump(),"application/json");

            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",std::string(e.what())}}.dump(),"application/json");
            }
        });

        // POST /api/cancel
        svr.Post("/api/cancel", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            try {
                json body       = json::parse(req.body);
                std::string bid = body.value("booking_id", "");
                std::string uid = body.value("user_id",    "");

                json bookings = FileDB::read(BOOKINGS_PATH);
                int bidx      = FileDB::findIndex(bookings, "booking_id", bid);
                if (bidx < 0) {
                    res.status = 404;
                    res.set_content(json{{"success",false},{"message","Booking not found"}}.dump(),"application/json");
                    return;
                }

                if (bookings[bidx].value("user_id","") != uid) {
                    res.status = 403;
                    res.set_content(json{{"success",false},{"message","Not your booking"}}.dump(),"application/json");
                    return;
                }

                if (bookings[bidx].value("status","") == "Cancelled") {
                    res.status = 400;
                    res.set_content(json{{"success",false},{"message","Already cancelled"}}.dump(),"application/json");
                    return;
                }

                // Free seats in flights.json
                std::string fid  = bookings[bidx].value("flight_id",  "");
                std::string sid  = bookings[bidx].value("schedule_id","");
                std::string cls  = bookings[bidx].value("class",      "economy");
                std::vector<std::string> seats = bookings[bidx]["seats"].get<std::vector<std::string>>();

                json flights = FileDB::read(FLIGHTS_PATH);
                int fidx2 = FileDB::findIndex(flights, "flight_id", fid);
                if (fidx2 >= 0) {
                    for (auto& sched : flights[fidx2]["schedules"]) {
                        if (sched.value("schedule_id","") == sid) {
                            auto& booked = sched["seats"][cls]["booked"];
                            for (const auto& seat : seats) {
                                booked.erase(
                                    std::remove(booked.begin(), booked.end(), seat),
                                    booked.end()
                                );
                            }
                            break;
                        }
                    }
                    FileDB::write(FLIGHTS_PATH, flights);
                }

                // Mark cancelled
                bookings[bidx]["status"] = "Cancelled";
                FileDB::write(BOOKINGS_PATH, bookings);

                res.set_content(json{{"success",true},{"message","Booking cancelled"}}.dump(),"application/json");

            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json{{"success",false},{"message",std::string(e.what())}}.dump(),"application/json");
            }
        });

        // GET /api/bookings?user_id=xxx
        svr.Get("/api/bookings", [](const httplib::Request& req, httplib::Response& res) {
            addCORS(res);
            std::string uid = req.get_param_value("user_id");
            json allBookings = FileDB::read(BOOKINGS_PATH);

            if (uid.empty()) {
                res.set_content(json{{"success",true},{"bookings",allBookings}}.dump(),"application/json");
                return;
            }

            json userBookings = json::array();
            for (const auto& b : allBookings) {
                if (b.value("user_id","") == uid) {
                    // Enrich with flight info
                    json bk = b;
                    json flights = FileDB::read("../data/flights.json");
                    int fidx = FileDB::findIndex(flights, "flight_id", b.value("flight_id",""));
                    if (fidx >= 0) {
                        bk["flight_info"] = {
                            {"airline",     flights[fidx].value("airline","MatAir")},
                            {"source",      flights[fidx].value("source","")},
                            {"destination", flights[fidx].value("destination","")}
                        };
                        // Find schedule departure
                        for (const auto& s : flights[fidx]["schedules"]) {
                            if (s.value("schedule_id","") == b.value("schedule_id","")) {
                                bk["flight_info"]["departure_time"] = s.value("departure_time","");
                                bk["flight_info"]["arrival_time"]   = s.value("arrival_time","");
                                bk["flight_info"]["aircraft"]       = s.value("aircraft","");
                                bk["flight_info"]["status"]         = s.value("status","");
                                break;
                            }
                        }
                    }
                    userBookings.push_back(bk);
                }
            }
            res.set_content(json{{"success",true},{"bookings",userBookings}}.dump(),"application/json");
        });
    }
}
