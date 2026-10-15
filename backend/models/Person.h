#pragma once
#include <string>
#include <vector>
#include "../include/json.hpp"
using json = nlohmann::json;

// â”€â”€â”€ Abstract Base: Person â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class Person {
protected:
    std::string user_id;
    std::string username;
    std::string email;
    std::string password;
public:
    Person() = default;
    Person(const std::string& id, const std::string& uname,
           const std::string& mail, const std::string& pw)
        : user_id(id), username(uname), email(mail), password(pw) {}

    virtual ~Person() = default;

    // Pure virtual â€” must be overridden
    virtual std::string getRole() const = 0;
    virtual json toJson() const = 0;

    // Getters
    const std::string& getId()       const { return user_id; }
    const std::string& getUsername() const { return username; }
    const std::string& getEmail()    const { return email; }
    const std::string& getPassword() const { return password; }

    bool checkPassword(const std::string& pw) const { return password == pw; }
};
