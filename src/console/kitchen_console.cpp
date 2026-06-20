#include "kitchen_console.hpp"

#include "console_io.hpp"
#include "console_views.hpp"
#include "ui_helpers.hpp"
#include "../modules/kitchen_fulfillment/kitchen_fulfillment_service.hpp"
#include "../shared/utils.hpp"

#include <iostream>

namespace app::console {

void kitchenLoop(FileDatabase &database, std::string station) {
    station = toLower(station);
    if (station != "kitchen" && station != "bar") {
        station = "kitchen";
    }

    while (true) {
        database.loadOrSeed();
        printTitle("FULFILLMENT WORKSPACE - " + station, "Suggested flow: view tasks -> start task -> mark task ready/report issue.");
        std::cout << "1. View tasks\n";
        std::cout << "2. Start task\n";
        std::cout << "3. Mark task ready\n";
        std::cout << "4. Report kitchen issue\n";
        std::cout << "5. Mark served\n";
        std::cout << "0. Exit\n";

        const int choice = readInt("Choose action: ");
        if (choice == 0) {
            return;
        }
        if (choice == 1) {
            printKitchenTasks(database, station);
        } else if (choice == 2) {
            printKitchenTasks(database, station);
            const int taskId = readInt("Start task id: ");
            printResult(kitchen_fulfillment::startTask(database, taskId, station, station));
        } else if (choice == 3) {
            printKitchenTasks(database, station);
            const int taskId = readInt("Ready task id: ");
            printResult(kitchen_fulfillment::completeTask(database, taskId, station, station));
        } else if (choice == 4) {
            printKitchenTasks(database, station);
            const int taskId = readInt("Issue task id: ");
            const std::string reason = readText("Issue reason: ");
            printResult(kitchen_fulfillment::reportIssue(database, taskId, station, reason, station));
        } else if (choice == 5) {
            printKitchenTasks(database, station);
            const int taskId = readInt("Served task id: ");
            printResult(kitchen_fulfillment::markServed(database, taskId, "waiter"));
        }
    }
}

}
