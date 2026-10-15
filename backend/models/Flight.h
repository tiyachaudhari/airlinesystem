#pragma once
#include <string>
#include <vector>
#include "../include/json.hpp"
using json = nlohmann::json;

// â”€â”€â”€ FlightSchedule â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
struct SeatMap {
    int total;
    std::vector<std::string> booked;
};

struct PriceMap {
    double economy;
    double business;
    double first;
};

struct FlightSchedule {
    std::string schedule_id;
    std::string departure_time;
    std::string arrival_time;
    int         duration_mins;
    std::string aircraft;
    std::string status;   // On Time / Delayed / Cancelled
    SeatMap     seats_economy;
    SeatMap     seats_business;
    SeatMap     seats_first;
    PriceMap    price;

    // Dynamic pricing: occupancy-based surge
    double dynamicPrice(const std::string& cls) const {
        const SeatMap* sm = nullptr;
        double base = 0;
        if (cls == "economy")  { sm = &seats_economy;  base = price.economy; }
        if (cls == "business") { sm = &seats_business; base = price.business; }
        if (cls == "first")    { sm = &seats_first;    base = price.first; }
        if (!sm || sm->total == 0) return base;

        double occ = (double)sm->booked.size() / sm->total;
        if      (occ < 0.30) return base * 0.90;
        else if (occ < 0.60) return base * 1.00;
        else if (occ < 0.80) return base * 1.20;
        else                  return base * 1.50;
    }

    int availableSeats(const std::string& cls) const {
        if (cls == "economy")  return seats_economy.total  - (int)seats_economy.booked.size();
        if (cls == "business") return seats_business.total - (int)seats_business.booked.size();
        if (cls == "first")    return seats_first.total    - (int)seats_first.booked.size();
        return 0;
    }

    static FlightSchedule fromJson(const json& j) {
        FlightSchedule s;
        s.schedule_id    = j.value("schedule_id",    "");
        s.departure_time = j.value("departure_time", "");
        s.arrival_time   = j.value("arrival_time",   "");
        s.duration_mins  = j.value("duration_mins",  0);
        s.aircraft       = j.value("aircraft",       "");
        s.status         = j.value("status",         "On Time");

        auto parseSeat = [&](const std::string& key) -> SeatMap {
            SeatMap sm;
            if (j.contains("seats") && j["seats"].contains(key)) {
                sm.total  = j["seats"][key].value("total", 0);
                sm.booked = j["seats"][key].value("booked", std::vector<std::string>{});
            }
            return sm;
        };
        s.seats_economy  = parseSeat("economy");
        s.seats_business = parseSeat("business");
        s.seats_first    = parseSeat("first");

        if (j.contains("price")) {
            s.price.economy  = j["price"].value("economy",  0.0);
            s.price.business = j["price"].value("business", 0.0);
            s.price.first    = j["price"].value("first",    0.0);
        }
        return s;
    }

    json toJson() const {
        return json{
            {"schedule_id",    schedule_id},
            {"departure_time", departure_time},
            {"arrival_time",   arrival_time},
            {"duration_mins",  duration_mins},
            {"aircraft",       aircraft},
            {"status",         status},
            {"seats", {
                {"economy",  {{"total", seats_economy.total},  {"booked", seats_economy.booked}}},
                {"business", {{"total", seats_business.total}, {"booked", seats_business.booked}}},
                {"first",    {{"total", seats_first.total},    {"booked", seats_first.booked}}}
            }},
            {"price", {
                {"economy",  price.economy},
                {"business", price.business},
                {"first",    price.first}
            }},
            {"dynamic_price", {
                {"economy",  dynamicPrice("economy")},
                {"business", dynamicPrice("business")},
                {"first",    dynamicPrice("first")}
            }},
            {"available_seats", {
                {"economy",  availableSeats("economy")},
                {"business", availableSeats("business")},
                {"first",    availableSeats("first")}
            }}
        };
    }
};

// â”€â”€â”€ Flight â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class Flight {
public:
    std::string flight_id;
    std::string airline;
    std::string source;
    std::string destination;
    std::vector<FlightSchedule> schedules;

    Flight() = default;

    static Flight fromJson(const json& j) {
        Flight f;
        f.flight_id   = j.value("flight_id",   "");
        f.airline     = j.value("airline",      "MatAir");
        f.source      = j.value("source",       "");
        f.destination = j.value("destination",  "");
        if (j.contains("schedules") && j["schedules"].is_array()) {
            for (const auto& s : j["schedules"]) {
                f.schedules.push_back(FlightSchedule::fromJson(s));
            }
        }
        return f;
    }

    json toJson() const {
        json schedArr = json::array();
        for (const auto& s : schedules) schedArr.push_back(s.toJson());
        return json{
            {"flight_id",   flight_id},
            {"airline",     airline},
            {"source",      source},
            {"destination", destination},
            {"schedules",   schedArr}
        };
    }
};
