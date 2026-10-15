#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../include/json.hpp"
using json = nlohmann::json;

// â”€â”€â”€ Passenger â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
struct Passenger {
    std::string name;
    int         age;
    std::string gender;

    json toJson() const {
        return json{{"name", name}, {"age", age}, {"gender", gender}};
    }
    static Passenger fromJson(const json& j) {
        return {j.value("name",""), j.value("age",0), j.value("gender","")};
    }
};

// â”€â”€â”€ Abstract Booking â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class Booking {
protected:
    std::string booking_id;
    std::string user_id;
    std::string flight_id;
    std::string schedule_id;
    std::string travel_date;
    std::string booked_at;
    std::string status;         // Confirmed / Cancelled
    std::vector<std::string> seats;
    std::vector<Passenger>   passengers;
    double total_price;

public:
    Booking() = default;
    Booking(const std::string& bid, const std::string& uid,
            const std::string& fid, const std::string& sid,
            const std::string& date, const std::string& bookedAt,
            const std::vector<std::string>& s, const std::vector<Passenger>& p)
        : booking_id(bid), user_id(uid), flight_id(fid), schedule_id(sid),
          travel_date(date), booked_at(bookedAt), status("Confirmed"),
          seats(s), passengers(p), total_price(0) {}

    virtual ~Booking() = default;

    // Pure virtuals â€” polymorphism
    virtual std::string getClass()          const = 0;
    virtual double calculatePrice(double base_price_per_seat) = 0;
    virtual std::string getSeatPrefix()     const = 0;

    // Common getters
    const std::string& getId()         const { return booking_id; }
    const std::string& getUserId()     const { return user_id; }
    const std::string& getFlightId()   const { return flight_id; }
    const std::string& getScheduleId() const { return schedule_id; }
    const std::string& getStatus()     const { return status; }
    double             getTotalPrice() const { return total_price; }
    void setStatus(const std::string& s)      { status = s; }

    virtual json toJson() const {
        json pax = json::array();
        for (const auto& p : passengers) pax.push_back(p.toJson());
        return json{
            {"booking_id",   booking_id},
            {"user_id",      user_id},
            {"flight_id",    flight_id},
            {"schedule_id",  schedule_id},
            {"class",        getClass()},
            {"travel_date",  travel_date},
            {"booked_at",    booked_at},
            {"status",       status},
            {"seats",        seats},
            {"passengers",   pax},
            {"total_price",  total_price}
        };
    }
};

// â”€â”€â”€ Economy Booking â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class EconomyBooking : public Booking {
public:
    using Booking::Booking;
    std::string getClass()      const override { return "economy"; }
    std::string getSeatPrefix() const override { return "E"; }

    // Economy: base price Ã— 1.0, with group discount (>3 pax â†’ 5% off)
    double calculatePrice(double base_price_per_seat) override {
        int count     = (int)passengers.size();
        double disc   = (count > 3) ? 0.95 : 1.0;
        total_price   = base_price_per_seat * count * disc;
        return total_price;
    }
};

// â”€â”€â”€ Business Booking â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class BusinessBooking : public Booking {
public:
    using Booking::Booking;
    std::string getClass()      const override { return "business"; }
    std::string getSeatPrefix() const override { return "B"; }

    // Business: base price Ã— 2.5 (encoded in flights.json already)
    double calculatePrice(double base_price_per_seat) override {
        total_price = base_price_per_seat * passengers.size();
        return total_price;
    }
};

// â”€â”€â”€ First Class Booking â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class FirstClassBooking : public Booking {
public:
    using Booking::Booking;
    std::string getClass()      const override { return "first"; }
    std::string getSeatPrefix() const override { return "F"; }

    // First: base price Ã— 5.0, always includes complimentary lounge (+500)
    double calculatePrice(double base_price_per_seat) override {
        total_price = (base_price_per_seat + 500.0) * passengers.size();
        return total_price;
    }
};

// â”€â”€â”€ Factory â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
inline std::unique_ptr<Booking> makeBooking(
    const std::string& cls,
    const std::string& bid, const std::string& uid,
    const std::string& fid, const std::string& sid,
    const std::string& date, const std::string& bookedAt,
    const std::vector<std::string>& seats,
    const std::vector<Passenger>& passengers)
{
    if (cls == "business")
        return std::make_unique<BusinessBooking>(bid, uid, fid, sid, date, bookedAt, seats, passengers);
    if (cls == "first")
        return std::make_unique<FirstClassBooking>(bid, uid, fid, sid, date, bookedAt, seats, passengers);
    return std::make_unique<EconomyBooking>(bid, uid, fid, sid, date, bookedAt, seats, passengers);
}

// â”€â”€â”€ Plain booking deserialization (for reading from JSON file) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
inline json bookingFromRawJson(const json& j) { return j; }
