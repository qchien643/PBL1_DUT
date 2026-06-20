#include "customer_console.hpp"

#include "console_io.hpp"
#include "console_views.hpp"
#include "ui_helpers.hpp"
#include "../modules/order_management/order_management_service.hpp"
#include "../modules/payment_billing/payment_billing_service.hpp"
#include "../policies/business_policies.hpp"

#include <iostream>
#include <utility>
#include <vector>

namespace app::console {

namespace {

int cartTotal(FileDatabase &database, const std::vector<order_management::CartLine> &cart) {
    int total = 0;
    for (const order_management::CartLine &cartItem : cart) {
        MenuItemRecord *item = database.findMenuItemById(cartItem.first);
        if (item != nullptr) {
            total += item->price * cartItem.second;
        }
    }
    return total;
}

int cartQuantity(const std::vector<order_management::CartLine> &cart) {
    int quantity = 0;
    for (const order_management::CartLine &cartItem : cart) {
        quantity += cartItem.second;
    }
    return quantity;
}

}

void customerLoop(FileDatabase &database, const std::string &tableCode) {
    std::vector<order_management::CartLine> cart;

    while (true) {
        database.loadOrSeed();
        SessionRecord *session = database.findActiveSessionByTableCode(tableCode);
        printTitle("CUSTOMER TABLE " + tableCode, "Cart: " + std::to_string(cartQuantity(cart)) + " item(s) | " + formatMoney(cartTotal(database, cart)));
        if (session == nullptr) {
            std::cout << "No active dining session. Please ask staff to open this table.\n";
        } else {
            std::cout << "Session #" << session->id << " | Status: " << session->status << '\n';
        }

        std::cout << "1. View menu\n";
        std::cout << "2. View recommendations\n";
        std::cout << "3. Add item to cart\n";
        std::cout << "4. View cart\n";
        std::cout << "5. Submit order\n";
        std::cout << "6. View order status\n";
        std::cout << "7. Request cancel ordered item\n";
        std::cout << "8. Request bill\n";
        std::cout << "9. Resolve sold-out order decision\n";
        std::cout << "0. Exit\n";

        const int choice = readInt("Choose action: ");
        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            printMenuItems(database);
        } else if (choice == 2) {
            if (session == nullptr) {
                std::cout << "Open a dining session first.\n";
            } else {
                printRecommendations(database, *session);
            }
        } else if (choice == 3) {
            printMenuItems(database);
            const int itemId = readInt("Menu item id: ");
            const int quantity = readInt("Quantity: ");
            MenuItemRecord *item = database.findMenuItemById(itemId);
            if (item == nullptr || quantity <= 0 || !policy::canOrderItem(*item)) {
                std::cout << "[WARN] Item is not orderable. Please choose an AVAILABLE item ID.\n";
                continue;
            }
            cart.push_back({itemId, quantity});
            std::cout << "[OK] Added " << item->name << " x" << quantity << " to cart.\n";
        } else if (choice == 4) {
            printTitle("CURRENT CART", "These items are not sent until you submit order.");
            int total = 0;
            for (const order_management::CartLine &cartItem : cart) {
                MenuItemRecord *item = database.findMenuItemById(cartItem.first);
                if (item == nullptr) {
                    continue;
                }
                const int lineTotal = item->price * cartItem.second;
                total += lineTotal;
                std::cout << item->name << " x" << cartItem.second << " = " << formatMoney(lineTotal) << '\n';
            }
            if (cart.empty()) {
                printEmpty("Cart is empty. Add menu items first.");
            }
            std::cout << "Cart total: " << formatMoney(total) << '\n';
        } else if (choice == 5) {
            if (session == nullptr) {
                std::cout << "[WARN] No active session.\n";
                continue;
            }
            const OperationResult result = order_management::submitOrder(database, session->id, cart, "customer");
            if (result.ok) {
                cart.clear();
            }
            printResult(result);
        } else if (choice == 6) {
            if (session == nullptr) {
                std::cout << "[WARN] No active session.\n";
            } else {
                printSessionOrders(database, *session);
            }
        } else if (choice == 7) {
            if (session == nullptr) {
                std::cout << "[WARN] No active session.\n";
                continue;
            }
            printSessionOrders(database, *session);
            const int orderItemId = readInt("Order item id to cancel: ");
            printResult(order_management::requestCancel(database, session->id, orderItemId, "customer"));
        } else if (choice == 8) {
            if (session == nullptr) {
                std::cout << "[WARN] No active session.\n";
                continue;
            }
            const OperationResult result = payment_billing::createBill(database, session->id, "customer");
            printResult(result);
            if (result.ok) {
                BillRecord *bill = nullptr;
                for (BillRecord &candidate : database.bills) {
                    if (candidate.id == result.id) {
                        bill = &candidate;
                        break;
                    }
                }
                if (bill != nullptr) {
                    std::cout << "Current total: " << formatMoney(bill->total) << '\n';
                }
            }
        } else if (choice == 9) {
            if (session == nullptr) {
                std::cout << "[WARN] No active session.\n";
                continue;
            }
            printSessionOrders(database, *session);
            const int orderId = readInt("Order id needing customer confirmation: ");
            const std::string decision = readText("Decision (CANCEL_ORDER/REMOVE_UNAVAILABLE/REPLACE_ITEMS): ");
            std::vector<order_management::CartLine> replacements;
            if (decision == "REPLACE_ITEMS") {
                const int itemId = readInt("Replacement menu item id: ");
                const int quantity = readInt("Replacement quantity: ");
                replacements.push_back({itemId, quantity});
            }
            printResult(order_management::resolveCustomerDecision(database, orderId, decision, replacements, "customer"));
        }
    }
}

}
