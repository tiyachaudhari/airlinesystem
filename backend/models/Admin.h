#pragma once
#include "Person.h"

// â”€â”€â”€ Admin (inherits Person) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class Admin : public Person {
public:
    Admin() = default;
    Admin(const std::string& id, const std::string& uname,
          const std::string& mail, const std::string& pw)
        : Person(id, uname, mail, pw) {}

    std::string getRole() const override { return "admin"; }

    // Admin-specific capability check
    bool canManageFlights()  const { return true; }
    bool canViewAllBookings() const { return true; }

    json toJson() const override {
        return json{
            {"user_id",  user_id},
            {"username", username},
            {"email",    email},
            {"password", password},
            {"role",     "admin"},
            {"bookings", json::array()}
        };
    }

    static Admin fromJson(const json& j) {
        return Admin(
            j.value("user_id",  ""),
            j.value("username", ""),
            j.value("email",    ""),
            j.value("password", "")
        );
    }
};
