#pragma once
#include <string>
#include <fstream>
#include <stdexcept>
#include "../include/json.hpp"
using json = nlohmann::json;

// â”€â”€â”€ FileDB â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Simple helper to read/write JSON flat files
namespace FileDB {

    inline json read(const std::string& filepath) {
        std::ifstream f(filepath);
        if (!f.is_open()) {
            // Return empty array if file missing
            return json::array();
        }
        try {
            json data;
            f >> data;
            return data;
        } catch (...) {
            return json::array();
        }
    }

    inline bool write(const std::string& filepath, const json& data) {
        std::ofstream f(filepath);
        if (!f.is_open()) return false;
        f << data.dump(2);
        return true;
    }

    // Find index of item by key=value
    inline int findIndex(const json& arr, const std::string& key, const std::string& val) {
        for (int i = 0; i < (int)arr.size(); ++i) {
            if (arr[i].contains(key) && arr[i][key].get<std::string>() == val) {
                return i;
            }
        }
        return -1;
    }

    // Case-insensitive string compare
    inline bool iequal(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
                return false;
        }
        return true;
    }
}
