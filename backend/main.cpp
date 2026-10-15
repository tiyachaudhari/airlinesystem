#include <iostream>
#include <fstream>
#include <string>
#include "include/httplib.h"
#include "routes/AuthRoutes.h"
#include "routes/FlightRoutes.h"
#include "routes/BookingRoutes.h"
#include "routes/AdminRoutes.h"

int main() {
    httplib::Server svr;

    // Enable CORS for preflight requests
    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200;
    });

    // Setup API routes
    AuthRoutes::setup(svr);
    FlightRoutes::setup(svr);
    BookingRoutes::setup(svr);
    AdminRoutes::setup(svr);

    // Serve static files from the frontend directory
    std::string base_dir = "../frontend";
    
    // Set mount point for frontend files
    if (!svr.set_mount_point("/", base_dir)) {
        std::cerr << "Cannot mount frontend directory!" << std::endl;
        return 1;
    }

    // Default route to index.html if root is requested
    svr.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        std::ifstream f(base_dir + "/index.html");
        if (f.good()) {
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("Index.html not found", "text/plain");
        }
    });

    // Handle generic fallback for HTML files to allow clean URLs (e.g., /results -> /results.html)
    svr.Get(R"(/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string page = req.matches[1];
        std::ifstream f(base_dir + "/" + page + ".html");
        if (f.good()) {
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            // Serve 404 naturally or let it fall through
            res.status = 404;
            res.set_content("Page not found", "text/plain");
        }
    });

    std::cout << "Starting MatAir Reservation Server..." << std::endl;
    std::cout << "Listening on http://localhost:8080" << std::endl;

    svr.listen("0.0.0.0", 8080);

    return 0;
}
