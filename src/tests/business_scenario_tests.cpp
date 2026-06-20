#include "business_scenario_tests.hpp"

#include "../infrastructure/file_database.hpp"
#include "../modules/kitchen_fulfillment/kitchen_fulfillment_service.hpp"
#include "../modules/menu_inventory/menu_inventory_service.hpp"
#include "../modules/order_management/order_management_service.hpp"
#include "../modules/payment_billing/payment_billing_service.hpp"
#include "../modules/table_session/table_session_service.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace app::tests {

namespace {

struct TestRunner {
    int failed{};

    void expect(bool condition, const std::string &message) {
        if (condition) {
            std::cout << "[PASS] " << message << '\n';
            return;
        }
        ++failed;
        std::cout << "[FAIL] " << message << '\n';
    }
};

FileDatabase freshDatabase(const std::string &name) {
    std::filesystem::create_directories("data");
    const std::string path = "data/" + name;
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    FileDatabase database(path);
    database.seed();
    database.save();
    return database;
}

void cleanupTestFiles() {
    const std::vector<std::string> files = {
        "data/test_double_submit.txt",
        "data/test_sold_out_decision.txt",
        "data/test_kitchen_issue.txt",
        "data/test_billing_payment.txt",
        "data/test_permission.txt",
    };
    std::error_code ignored;
    for (const std::string &file : files) {
        std::filesystem::remove(file, ignored);
    }
}

SessionRecord *openDemoTable(FileDatabase &database, const std::string &tableCode) {
    OperationResult opened = table_session::openTable(database, tableCode, "cashier");
    if (!opened.ok) {
        return nullptr;
    }
    return database.findSessionById(opened.id);
}

KitchenTaskRecord *firstTaskForOrder(FileDatabase &database, int orderId) {
    for (KitchenTaskRecord &task : database.kitchenTasks) {
        OrderItemRecord *item = database.findOrderItemById(task.orderItemId);
        if (item != nullptr && item->orderId == orderId) {
            return &task;
        }
    }
    return nullptr;
}

void testDoubleSubmit(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_double_submit.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    runner.expect(session != nullptr, "open table for double submit");

    const std::vector<order_management::CartLine> cart = {{1, 1}};
    OperationResult first = order_management::submitOrder(database, session->id, cart, "customer", "same-key");
    OperationResult second = order_management::submitOrder(database, session->id, cart, "customer", "same-key");

    runner.expect(first.ok, "first submit succeeds");
    runner.expect(second.ok && second.code == "IDEMPOTENT_REPLAY", "duplicate submit returns idempotent replay");
    runner.expect(first.id == second.id, "duplicate submit returns same order id");
    runner.expect(database.orders.size() == 1, "duplicate submit does not create another order");
}

void testSoldOutDecisionAndReplacement(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_sold_out_decision.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    const std::vector<order_management::CartLine> cart = {{1, 1}};

    OperationResult submitted = order_management::submitOrder(database, session->id, cart, "customer", "soldout-key");
    menu_inventory::setAvailability(database, 1, "SOLD_OUT", "manager");
    OperationResult accepted = order_management::acceptOrder(database, submitted.id, "cashier");
    OrderRecord *order = database.findOrderById(submitted.id);

    runner.expect(accepted.ok && accepted.code == "UNAVAILABLE_ITEM_REQUIRES_CUSTOMER_CONFIRMATION", "sold-out accept asks customer confirmation");
    runner.expect(order != nullptr && order->status == "NEEDS_CUSTOMER_CONFIRMATION", "order waits for customer decision");

    OperationResult decision = order_management::resolveCustomerDecision(database, submitted.id, "REPLACE_ITEMS", {{2, 1}}, "customer");
    OperationResult acceptedAgain = order_management::acceptOrder(database, submitted.id, "cashier");

    runner.expect(decision.ok, "customer replacement decision succeeds");
    runner.expect(acceptedAgain.ok, "cashier accepts replaced order");
    runner.expect(firstTaskForOrder(database, submitted.id) != nullptr, "replacement accept creates kitchen task");
}

void testKitchenIssueBlocksAndResolvesBill(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_kitchen_issue.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{2, 1}}, "customer", "issue-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *task = firstTaskForOrder(database, submitted.id);

    kitchen_fulfillment::startTask(database, task->id, task->station, task->station);
    OperationResult issue = kitchen_fulfillment::reportIssue(database, task->id, task->station, "ingredient unavailable", task->station);
    OperationResult billBlocked = payment_billing::createBill(database, session->id, "customer");
    OperationResult resolved = kitchen_fulfillment::resolveIssue(database, issue.id, "CANCEL_ITEM", "cashier");
    OperationResult billAllowed = payment_billing::createBill(database, session->id, "customer");

    runner.expect(issue.ok, "kitchen can report issue");
    runner.expect(!billBlocked.ok && billBlocked.code == "BILL_BLOCKED_BY_ACTIVE_WORK", "kitchen issue blocks bill");
    runner.expect(resolved.ok, "cashier resolves kitchen issue");
    runner.expect(billAllowed.ok, "bill can be created after issue resolution");
}

void testBillingPaymentAndReopen(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_billing_payment.txt");
    SessionRecord *session = openDemoTable(database, "T02");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{2, 1}}, "customer", "bill-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *task = firstTaskForOrder(database, submitted.id);
    kitchen_fulfillment::startTask(database, task->id, task->station, task->station);
    kitchen_fulfillment::completeTask(database, task->id, task->station, task->station);

    OperationResult billCreated = payment_billing::createBill(database, session->id, "customer");
    BillRecord *bill = database.findBillById(billCreated.id);
    OperationResult insufficient = payment_billing::confirmPayment(database, bill->id, "cash", bill->total - 1, bill->version, "pay-low", "cashier");
    database.touchSession(session->id);
    OperationResult stale = payment_billing::confirmPayment(database, bill->id, "cash", bill->total, bill->version, "pay-stale", "cashier");
    bill->status = "OPEN";
    bill->sessionVersion = session->version;
    OperationResult reopened = payment_billing::reopenBill(database, bill->id, "cashier");

    runner.expect(billCreated.ok && bill != nullptr, "bill is created after ready task");
    runner.expect(!insufficient.ok && insufficient.code == "PAYMENT_AMOUNT_INVALID", "payment rejects insufficient amount");
    runner.expect(!stale.ok && stale.code == "BILL_STALE_RECALCULATE_REQUIRED", "payment rejects stale bill");
    runner.expect(reopened.ok, "cashier can reopen open bill");
}

void testPermission(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_permission.txt");
    SessionRecord *session = openDemoTable(database, "T03");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{3, 1}}, "customer", "perm-key");
    OperationResult customerAccept = order_management::acceptOrder(database, submitted.id, "customer");
    OperationResult kitchenPay = payment_billing::confirmPayment(database, 999, "cash", 1000, 1, "bad-pay", "kitchen");

    runner.expect(!customerAccept.ok && customerAccept.code == "PERMISSION_DENIED", "customer cannot accept order");
    runner.expect(!kitchenPay.ok && kitchenPay.code == "PERMISSION_DENIED", "kitchen cannot confirm payment");
}

}

int runBusinessScenarioTests() {
    TestRunner runner;
    testDoubleSubmit(runner);
    testSoldOutDecisionAndReplacement(runner);
    testKitchenIssueBlocksAndResolvesBill(runner);
    testBillingPaymentAndReopen(runner);
    testPermission(runner);

    cleanupTestFiles();
    if (runner.failed == 0) {
        std::cout << "All business scenario tests passed.\n";
        return 0;
    }
    std::cout << runner.failed << " business scenario test(s) failed.\n";
    return 1;
}

}
