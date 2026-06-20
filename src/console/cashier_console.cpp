#include "cashier_console.hpp"

#include "console_io.hpp"
#include "console_views.hpp"
#include "ui_helpers.hpp"
#include "../modules/order_management/order_management_service.hpp"
#include "../modules/payment_billing/payment_billing_service.hpp"
#include "../modules/kitchen_fulfillment/kitchen_fulfillment_service.hpp"
#include "../modules/table_session/table_session_service.hpp"

#include <iostream>

namespace app::console {

void cashierLoop(FileDatabase &database) {
    while (true) {
        database.loadOrSeed();
        printTitle("CASHIER WORKSPACE", "Suggested flow: open table -> approve order -> create bill -> confirm payment.");
        std::cout << "1. View tables\n";
        std::cout << "2. Open table/session\n";
        std::cout << "3. Merge tables\n";
        std::cout << "4. Transfer table\n";
        std::cout << "5. View pending orders\n";
        std::cout << "6. Accept order\n";
        std::cout << "7. Reject order\n";
        std::cout << "8. View cancel requests\n";
        std::cout << "9. Approve cancel request\n";
        std::cout << "10. Create bill\n";
        std::cout << "11. Confirm payment\n";
        std::cout << "12. Mark table cleaned\n";
        std::cout << "13. Resolve kitchen issue\n";
        std::cout << "14. Reopen bill\n";
        std::cout << "0. Exit\n";

        const int choice = readInt("Choose action: ");
        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            printTables(database);
        } else if (choice == 2) {
            printTables(database);
            const std::string tableCode = readText("Open table code (ex: T01): ");
            printResult(table_session::openTable(database, tableCode, "cashier"));
        } else if (choice == 3) {
            printTables(database);
            const std::string mainTableCode = readText("Main table code with active session (ex: T01): ");
            const std::string joinedTableCode = readText("Table to merge into that session (ex: T02): ");
            printResult(table_session::mergeTables(database, mainTableCode, joinedTableCode, "cashier"));
        } else if (choice == 4) {
            printTables(database);
            const std::string sourceCode = readText("Current occupied table code (ex: T01): ");
            const std::string targetCode = readText("Target available table code (ex: T03): ");
            printResult(table_session::transferTable(database, sourceCode, targetCode, "cashier"));
        } else if (choice == 5) {
            printPendingOrders(database);
        } else if (choice == 6) {
            printPendingOrders(database);
            const int orderId = readInt("Accept order id: ");
            printResult(order_management::acceptOrder(database, orderId, "cashier"));
        } else if (choice == 7) {
            printPendingOrders(database);
            const int orderId = readInt("Reject order id: ");
            printResult(order_management::rejectOrder(database, orderId, "cashier"));
        } else if (choice == 8) {
            printCancelRequests(database);
        } else if (choice == 9) {
            printCancelRequests(database);
            const int orderItemId = readInt("Approve cancel order item id: ");
            printResult(order_management::approveCancel(database, orderItemId, "cashier"));
        } else if (choice == 10) {
            printTables(database);
            const std::string tableCode = readText("Bill table code (ex: T01): ");
            const OperationResult result = payment_billing::createBillForTable(database, tableCode, "cashier");
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
        } else if (choice == 11) {
            printOpenBills(database);
            const int billId = readInt("Paid bill id: ");
            const std::string paymentMethod = readText("Payment method (cash/card/qr): ");
            BillRecord *bill = database.findBillById(billId);
            const int paidAmount = readInt("Paid amount: ");
            printResult(payment_billing::confirmPayment(database, billId, paymentMethod, paidAmount, bill == nullptr ? 0 : bill->version, "cmd-pay-" + std::to_string(billId), "cashier"));
        } else if (choice == 12) {
            printTables(database);
            const std::string tableCode = readText("Cleaned table code (ex: T01): ");
            printResult(table_session::markTableCleaned(database, tableCode, "cashier"));
        } else if (choice == 13) {
            const int issueId = readInt("Kitchen issue id: ");
            const std::string resolution = readText("Resolution (CANCEL_ITEM/REMAKE_SAME_ITEM/KEEP_AND_BILL_WITH_MANAGER_OVERRIDE): ");
            printResult(kitchen_fulfillment::resolveIssue(database, issueId, resolution, "cashier"));
        } else if (choice == 14) {
            printOpenBills(database);
            const int billId = readInt("Reopen bill id: ");
            printResult(payment_billing::reopenBill(database, billId, "cashier"));
        }
    }
}

}
