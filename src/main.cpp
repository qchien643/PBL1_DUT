#include "console/cashier_console.hpp"
#include "console/console_io.hpp"
#include "console/customer_console.hpp"
#include "console/kitchen_console.hpp"
#include "console/manager_console.hpp"
#include "infrastructure/file_database.hpp"
#include "server/server_app.hpp"
#include "shared/utils.hpp"
#include "tests/business_scenario_tests.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    app::FileDatabase database("data/restaurant_db.txt");

    if (argc < 2) {
        app::console::printHelp();
        return 0;
    }

    const std::string mode = app::toLower(argv[1]);
    if (mode == "reset") {
        database.seed();
        if (!database.save()) {
            std::cerr << "Could not save demo data.\n";
            return 1;
        }
        std::cout << "Demo data reset.\n";
        return 0;
    }
    if (mode == "test") {
        return app::tests::runBusinessScenarioTests();
    }

    database.loadOrSeed();

    if (mode == "manager") {
        app::console::managerLoop(database);
    } else if (mode == "cashier") {
        app::console::cashierLoop(database);
    } else if (mode == "customer") {
        const std::string tableCode = argc >= 3 ? argv[2] : "T01";
        app::console::customerLoop(database, tableCode);
    } else if (mode == "kitchen") {
        const std::string station = argc >= 3 ? argv[2] : "kitchen";
        app::console::kitchenLoop(database, station);
    } else if (mode == "server") {
        const int port = argc >= 3 ? std::stoi(argv[2]) : 8080;
        return app::server::runWebServer(port);
    } else {
        app::console::printHelp();
    }

    return 0;
}
