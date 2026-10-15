#pragma once
#include "Person.h"
#include <vector>

// â”€â”€â”€ User (inherits Person) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class User : public Person {
private:
    std::vector<std::string> bookings;  // booking IDs
public:
    User() = default;
    User(const std::string& id, const std::string& uname,
         const std::string& mail, const std::string& pw,
         const std::vector<std::string>& bks = {})
        : Person(id, uname, mail, pw), bookings(bks) {}

    std::string getRole() const override { return "user"; }

    const std::vector<std::string>& getBookings() const { return bookings; }

    void addBooking(const std::string& bk_id) {
        bookings.push_back(bk_id);
    }
    void removeBooking(const std::string& bk_id) {
        bookings.erase(std::remove(bookings.begin(), bookings.end(), bk_id), bookings.end());
    }

    json toJson() const override {
        return json{
            {"user_id",  user_id},
            {"username", username},
            {"email",    email},
            {"password", password},
            {"role",     "user"},
            {"bookings", bookings}
        };
    }

    static User fromJson(const json& j) {
        std::vector<std::string> bks;
        if (j.contains("bookings") && j["bookings"].is_array()) {
            bks = j["bookings"].get<std::vector<std::string>>();
        }
        return User(
            j.value("user_id",  ""),
            j.value("username", ""),
            j.value("email",    ""),
            j.value("password", ""),
            bks
        );
    }
};
