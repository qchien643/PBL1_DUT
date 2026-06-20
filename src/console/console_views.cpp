#include "console_views.hpp"

#include "ui_helpers.hpp"
#include "../modules/kitchen_fulfillment/kitchen_fulfillment_service.hpp"
#include "../modules/menu_inventory/menu_inventory_service.hpp"
#include "../modules/recommendation_ai_ml/recommendation_service.hpp"
#include "../modules/table_session/table_session_service.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace app::console {

void printMenuItems(FileDatabase &database, bool includeHidden) {
    printTitle("MENU CATALOG", includeHidden ? "Manager view: includes hidden/inactive items." : "Customer view: only active items are shown.");
    std::cout << std::left << std::setw(4) << "ID" << std::setw(28) << "Name" << std::setw(12) << "Category"
              << std::setw(15) << "Price" << std::setw(12) << "Status" << "Station\n";
    printLine('-');
    bool hasRows = false;
    for (const MenuItemRecord &item : menu_inventory::visibleMenuItems(database, includeHidden)) {
        hasRows = true;
        std::cout << std::left << std::setw(4) << item.id << std::setw(28) << item.name << std::setw(12) << item.category
                  << std::setw(15) << formatMoney(item.price) << std::setw(12) << item.availabilityStatus << item.station << '\n';
    }
    if (!hasRows) {
        printEmpty("No menu item is available.");
    }
    printHint("Use the ID column when adding item to cart or changing availability.");
}

void printTables(FileDatabase &database) {
    printTitle("TABLE BOARD", "Legend: AVAILABLE=empty, OCCUPIED=serving, CLEANING=needs cleanup.");
    std::cout << std::left << std::setw(8) << "Code" << std::setw(14) << "State" << std::setw(16) << "Meaning"
              << std::setw(10) << "Session" << "Joined tables\n";
    printLine('-');
    for (TableRecord &table : database.tables) {
        std::string joinedTables = "-";
        if (table.activeSessionId != 0) {
            SessionRecord *session = database.findSessionById(table.activeSessionId);
            if (session != nullptr) {
                joinedTables = table_session::tableCodesForSession(database, *session);
            }
        }
        std::cout << std::left << std::setw(8) << table.code << std::setw(14) << table.state
                  << std::setw(16) << explainTableState(table.state) << std::setw(10)
                  << table.activeSessionId << joinedTables << '\n';
    }
    printHint("Cashier opens a table first, then customer CMD for that table can order.");
}

void printSessionOrders(FileDatabase &database, const SessionRecord &session) {
    printTitle("ORDER STATUS", "Session #" + std::to_string(session.id) + " | Tables " + table_session::tableCodesForSession(database, session));
    bool hasOrders = false;
    for (const OrderRecord &order : database.orders) {
        if (order.sessionId != session.id) {
            continue;
        }
        hasOrders = true;
        std::cout << "Order #" << order.id << " | " << order.status << " | " << order.createdAt << '\n';
        bool hasItems = false;
        for (const OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId != order.id) {
                continue;
            }
            hasItems = true;
            MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
            std::cout << "  Item #" << orderItem.id << " | "
                      << (menuItem == nullptr ? "Unknown item" : menuItem->name)
                      << " x" << orderItem.quantity << " | " << explainOrderItemStatus(orderItem.status)
                      << " | " << formatMoney(orderItem.quantity * orderItem.unitPrice);
            if (!orderItem.note.empty()) {
                std::cout << " | " << orderItem.note;
            }
            std::cout << '\n';
        }
        if (!hasItems) {
            printEmpty("This order has no item.");
        }
    }
    if (!hasOrders) {
        printEmpty("This table has not submitted any order yet.");
    }
    printHint("Use Item # when requesting cancellation.");
}

void printPendingOrders(FileDatabase &database) {
    printTitle("PENDING ORDERS", "Orders submitted by customers and waiting for cashier approval.");
    bool hasRows = false;
    for (const OrderRecord &order : database.orders) {
        if (order.status != "SUBMITTED") {
            continue;
        }
        hasRows = true;
        SessionRecord *session = database.findSessionById(order.sessionId);
        std::cout << "Order #" << order.id << " | Session " << order.sessionId
                  << " | Tables " << (session == nullptr ? "-" : table_session::tableCodesForSession(database, *session)) << '\n';
        for (const OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId == order.id) {
                MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
                std::cout << "  " << (menuItem == nullptr ? "Unknown item" : menuItem->name)
                          << " x" << orderItem.quantity << " | " << explainOrderItemStatus(orderItem.status) << '\n';
            }
        }
    }
    if (!hasRows) {
        printEmpty("No order is waiting. Ask customer to submit an order first.");
    }
    printHint("Accepting an order creates kitchen/bar tasks automatically.");
}

void printCancelRequests(FileDatabase &database) {
    printTitle("CANCEL REQUESTS", "Staff can approve only if kitchen/bar has not started the item.");
    bool hasRows = false;
    for (const OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.status != "CANCEL_REQUESTED") {
            continue;
        }
        hasRows = true;
        const OrderRecord *order = nullptr;
        for (const OrderRecord &candidate : database.orders) {
            if (candidate.id == orderItem.orderId) {
                order = &candidate;
                break;
            }
        }
        MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
        std::cout << "Order item #" << orderItem.id << " | Order #" << orderItem.orderId
                  << " | Session " << (order == nullptr ? 0 : order->sessionId)
                  << " | " << (menuItem == nullptr ? "Unknown item" : menuItem->name)
                  << " x" << orderItem.quantity << '\n';
    }
    if (!hasRows) {
        printEmpty("No customer is requesting cancellation.");
    }
}

void printOpenBills(FileDatabase &database) {
    printTitle("OPEN BILLS", "Bills waiting for cashier payment confirmation.");
    bool hasRows = false;
    for (const BillRecord &bill : database.bills) {
        if (bill.status == "OPEN") {
            hasRows = true;
            SessionRecord *session = database.findSessionById(bill.sessionId);
            std::cout << "Bill #" << bill.id << " | Session #" << bill.sessionId
                      << " | Tables " << (session == nullptr ? "-" : table_session::tableCodesForSession(database, *session))
                      << " | Total " << formatMoney(bill.total)
                      << " | Version " << bill.version << '\n';
        }
    }
    if (!hasRows) {
        printEmpty("No open bill right now.");
    }
}

void printKitchenTasks(FileDatabase &database, const std::string &station) {
    printTitle("FULFILLMENT BOARD - " + station, "Only accepted orders appear here.");
    bool hasRows = false;
    for (const KitchenTaskRecord &task : kitchen_fulfillment::activeTasksForStation(database, station)) {
        OrderItemRecord *orderItem = database.findOrderItemById(task.orderItemId);
        if (orderItem == nullptr) {
            continue;
        }
        hasRows = true;
        MenuItemRecord *menuItem = database.findMenuItemById(orderItem->menuItemId);
        OrderRecord *order = database.findOrderById(orderItem->orderId);
        SessionRecord *session = order == nullptr ? nullptr : database.findSessionById(order->sessionId);
        std::cout << "Task #" << task.id << " | " << task.status
                  << " | " << (menuItem == nullptr ? "Unknown item" : menuItem->name)
                  << " x" << orderItem->quantity
                  << " | Tables " << (session == nullptr ? "-" : table_session::tableCodesForSession(database, *session))
                  << " | Order item #" << orderItem->id;
        if (task.issueId != 0 || !task.issue.empty()) {
            std::cout << " | Issue #" << task.issueId << ": " << task.issue;
        }
        std::cout << '\n';
    }
    if (!hasRows) {
        printEmpty("No active task. Cashier must accept customer orders before tasks appear.");
    }
    printHint("Start a PENDING task, then mark it READY when finished.");
}

void printRecommendations(FileDatabase &database, const SessionRecord &session) {
    printTitle("RECOMMENDED FOR THIS TABLE", "AI/ML MVP: latent-factor ranking with best-seller fallback.");

    const std::vector<recommendation_ai_ml::Recommendation> recommendations = recommendation_ai_ml::recommendForSession(database, session, 3);
    if (recommendations.empty()) {
        printEmpty("No available recommendation right now.");
        return;
    }

    for (const recommendation_ai_ml::Recommendation &recommendation : recommendations) {
        MenuItemRecord *item = database.findMenuItemById(recommendation.itemId);
        if (item == nullptr) {
            continue;
        }
        const int matchPercent = std::min(99, 60 + static_cast<int>(recommendation.score * 25));
        std::cout << item->id << ". " << item->name << " | " << formatMoney(item->price)
                  << " | match " << matchPercent << "%\n";
    }
    printHint("Add by menu item ID if the customer wants one of these suggestions.");
}

}
