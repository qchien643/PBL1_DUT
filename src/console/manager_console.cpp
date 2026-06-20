#include "manager_console.hpp"

#include "console_io.hpp"
#include "console_views.hpp"
#include "ui_helpers.hpp"
#include "../modules/menu_inventory/menu_inventory_service.hpp"
#include "../modules/reporting_audit/reporting_audit_service.hpp"

#include <iostream>

namespace app::console {

void managerLoop(FileDatabase &database) {
    while (true) {
        database.loadOrSeed();
        printTitle("MANAGER WORKSPACE", "Manage menu availability, revenue, and audit trail.");
        std::cout << "1. View full menu\n";
        std::cout << "2. Mark item sold out\n";
        std::cout << "3. Mark item available\n";
        std::cout << "4. View paid revenue\n";
        std::cout << "5. View audit log\n";
        std::cout << "6. Reset demo data\n";
        std::cout << "0. Exit\n";

        const int choice = readInt("Choose action: ");
        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            printMenuItems(database, true);
        } else if (choice == 2 || choice == 3) {
            printMenuItems(database, true);
            const int itemId = readInt("Menu item id: ");
            const OperationResult result = menu_inventory::setAvailability(database, itemId, choice == 2 ? "SOLD_OUT" : "AVAILABLE", "manager");
            printResult(result);
        } else if (choice == 4) {
            std::cout << "Paid revenue: " << formatMoney(reporting_audit::paidRevenue(database)) << '\n';
        } else if (choice == 5) {
            printTitle("AUDIT LOG", "Last 20 important actions.");
            const std::vector<AuditEventRecord> events = reporting_audit::recentAuditEvents(database, 20);
            if (events.empty()) {
                printEmpty("No audit event yet.");
            }
            for (const AuditEventRecord &event : events) {
                std::cout << "#" << event.id << " | " << event.createdAt << " | " << event.role << " | " << event.message << '\n';
            }
        } else if (choice == 6) {
            const std::string confirm = readText("Type RESET to confirm: ");
            if (confirm == "RESET") {
                database.seed();
                database.save();
                std::cout << "Demo data reset.\n";
            }
        }
    }
}

}
