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
    std::filesystem::remove_all("data/" + std::filesystem::path(name).stem().string() + "_db", ignored);
    FileDatabase database(path);
    database.seed();
    database.save();
    return database;
}

void cleanupTestFiles() {
    const std::vector<std::string> files = {
        "data/test_double_submit.txt",
        "data/test_table_session_edges.txt",
        "data/test_merge_tables_moves_orders.txt",
        "data/test_submit_guards.txt",
        "data/test_sold_out_decision.txt",
        "data/test_cancel_pending_item_bill.txt",
        "data/test_cancel_preparing_item.txt",
        "data/test_kitchen_task_transitions.txt",
        "data/test_kitchen_issue.txt",
        "data/test_bill_requires_order.txt",
        "data/test_bill_waits_all_tasks.txt",
        "data/test_bill_lock_reopen.txt",
        "data/test_payment_success.txt",
        "data/test_billing_payment.txt",
        "data/test_permission.txt",
    };
    std::error_code ignored;
    for (const std::string &file : files) {
        std::filesystem::remove(file, ignored);
        std::filesystem::remove_all("data/" + std::filesystem::path(file).stem().string() + "_db", ignored);
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

std::vector<int> taskIdsForOrder(FileDatabase &database, int orderId) {
    std::vector<int> taskIds;
    for (const KitchenTaskRecord &task : database.kitchenTasks) {
        OrderItemRecord *item = database.findOrderItemById(task.orderItemId);
        if (item != nullptr && item->orderId == orderId) {
            taskIds.push_back(task.id);
        }
    }
    return taskIds;
}

int billLineCount(const FileDatabase &database, int billId) {
    int count = 0;
    for (const BillLineRecord &line : database.billLines) {
        if (line.billId == billId) {
            ++count;
        }
    }
    return count;
}

void startAndReadyTask(FileDatabase &database, int taskId) {
    KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
    if (task == nullptr) {
        return;
    }
    if (task->status == "PENDING") {
        kitchen_fulfillment::startTask(database, task->id, task->station, task->station);
    }
    task = database.findKitchenTaskById(taskId);
    if (task != nullptr && task->status == "PREPARING") {
        kitchen_fulfillment::completeTask(database, task->id, task->station, task->station);
    }
}

void testTableSessionEdges(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_table_session_edges.txt");

    OperationResult opened = table_session::openTable(database, "T01", "cashier");
    OperationResult duplicateOpen = table_session::openTable(database, "T01", "cashier");
    table_session::openTable(database, "T02", "cashier");
    OperationResult transferToOccupied = table_session::transferTable(database, "T01", "T02", "cashier");
    OperationResult transferToAvailable = table_session::transferTable(database, "T01", "T03", "cashier");
    TableRecord *sourceTable = database.findTableByCode("T01");
    TableRecord *targetTable = database.findTableByCode("T03");
    OperationResult cleaned = table_session::markTableCleaned(database, "T01", "cashier");

    runner.expect(opened.ok, "TS-01 cashier opens available table");
    runner.expect(!duplicateOpen.ok, "TS-02 opening occupied table is rejected");
    runner.expect(!transferToOccupied.ok, "TS-05 transfer to occupied table is rejected");
    runner.expect(transferToAvailable.ok, "TS-04 transfer to available table succeeds");
    runner.expect(sourceTable != nullptr && sourceTable->state == "AVAILABLE", "transferred source table can be marked available after cleaning");
    runner.expect(targetTable != nullptr && targetTable->state == "OCCUPIED" && targetTable->activeSessionId == opened.id, "transfer preserves active session on target table");
    runner.expect(cleaned.ok, "cleaning table can be returned to available");
}

void testMergeTablesMovesOrders(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_merge_tables_moves_orders.txt");
    SessionRecord *mainSession = openDemoTable(database, "T01");
    const int mainSessionId = mainSession == nullptr ? 0 : mainSession->id;
    SessionRecord *joinedSession = openDemoTable(database, "T02");
    const int joinedSessionId = joinedSession == nullptr ? 0 : joinedSession->id;
    OperationResult submitted = order_management::submitOrder(database, joinedSessionId, {{5, 1}}, "customer", "merge-order-key");

    OperationResult merged = table_session::mergeTables(database, "T01", "T02", "cashier");
    OrderRecord *order = database.findOrderById(submitted.id);
    SessionRecord *closedJoined = database.findSessionById(joinedSessionId);
    TableRecord *joinedTable = database.findTableByCode("T02");

    runner.expect(merged.ok, "TS-06 active tables can be merged");
    runner.expect(order != nullptr && order->sessionId == mainSessionId, "merge moves joined table orders to main session");
    runner.expect(closedJoined != nullptr && closedJoined->status == "CLOSED", "merge closes joined session");
    runner.expect(joinedTable != nullptr && joinedTable->activeSessionId == mainSessionId, "merge keeps joined physical table on main session");
}

void testSubmitGuards(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_submit_guards.txt");

    OperationResult noSession = order_management::submitOrder(database, 999, {{1, 1}}, "customer", "no-session-key");
    menu_inventory::setAvailability(database, 1, "SOLD_OUT", "manager");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult soldOutOnly = order_management::submitOrder(database, session->id, {{1, 1}}, "customer", "soldout-only-key");

    runner.expect(!noSession.ok && noSession.code == "SESSION_NOT_ACTIVE", "TS-03 customer cannot submit order without active session");
    runner.expect(!soldOutOnly.ok && soldOutOnly.code == "ORDER_NO_ORDERABLE_ITEM", "MI-02 sold-out-only cart is rejected at submit");
    runner.expect(database.orders.empty(), "invalid submit attempts do not create orders");
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

void testCancelPendingItemAndBillExcludesIt(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_cancel_pending_item_bill.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}, {2, 1}}, "customer", "cancel-pending-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    const std::vector<int> taskIds = taskIdsForOrder(database, submitted.id);
    KitchenTaskRecord *cancelledTask = taskIds.empty() ? nullptr : database.findKitchenTaskById(taskIds.front());
    KitchenTaskRecord *billableTask = taskIds.size() < 2 ? nullptr : database.findKitchenTaskById(taskIds[1]);

    OperationResult cancelRequest = cancelledTask == nullptr
        ? OperationResult::failure("Missing task")
        : order_management::requestCancel(database, session->id, cancelledTask->orderItemId, "customer");
    OperationResult approvedCancel = cancelledTask == nullptr
        ? OperationResult::failure("Missing task")
        : order_management::approveCancel(database, cancelledTask->orderItemId, "cashier");
    if (billableTask != nullptr) {
        startAndReadyTask(database, billableTask->id);
    }
    OperationResult billCreated = payment_billing::createBill(database, session->id, "customer");
    BillRecord *bill = database.findBillById(billCreated.id);
    const std::vector<KitchenTaskRecord> activeStationTasks = cancelledTask == nullptr
        ? std::vector<KitchenTaskRecord>{}
        : kitchen_fulfillment::activeTasksForStation(database, cancelledTask->station);
    bool cancelledTaskStillActive = false;
    for (const KitchenTaskRecord &task : activeStationTasks) {
        if (cancelledTask != nullptr && task.id == cancelledTask->id) {
            cancelledTaskStillActive = true;
        }
    }

    runner.expect(cancelRequest.ok, "OR-04 customer can request cancel for accepted item before kitchen starts");
    runner.expect(approvedCancel.ok, "OR-04 cashier can approve pending item cancellation");
    runner.expect(cancelledTask != nullptr && database.findKitchenTaskById(cancelledTask->id)->status == "CANCELLED", "approved cancel also cancels kitchen task");
    runner.expect(!cancelledTaskStillActive, "OR-04 cancelled kitchen/bar task is removed from active station board");
    runner.expect(billCreated.ok && bill != nullptr, "BP-03 bill can be created after remaining item is ready");
    runner.expect(bill != nullptr && bill->total == 79000 && billLineCount(database, bill->id) == 1, "BP-03 cancelled item is excluded from bill");
}

void testCancelPreparingItemIsDenied(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_cancel_preparing_item.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}}, "customer", "cancel-preparing-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *task = firstTaskForOrder(database, submitted.id);
    kitchen_fulfillment::startTask(database, task->id, task->station, task->station);

    OperationResult cancelRequest = order_management::requestCancel(database, session->id, task->orderItemId, "customer");
    OperationResult approveCancel = order_management::approveCancel(database, task->orderItemId, "cashier");
    OrderItemRecord *item = database.findOrderItemById(task->orderItemId);

    runner.expect(!cancelRequest.ok && cancelRequest.code == "ORDER_ITEM_CANCEL_NOT_ALLOWED", "OR-05 customer cannot cancel item after kitchen starts");
    runner.expect(!approveCancel.ok && approveCancel.code == "ORDER_ITEM_CANCEL_NOT_ALLOWED", "OR-05 cashier cannot approve normal cancel for preparing item");
    runner.expect(item != nullptr && item->status == "PREPARING" && task->status == "PREPARING", "preparing item/task remain active after denied cancel");
}

void testKitchenTaskTransitions(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_kitchen_task_transitions.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}, {5, 1}}, "customer", "task-transition-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *kitchenTask = nullptr;
    KitchenTaskRecord *barTask = nullptr;
    for (int taskId : taskIdsForOrder(database, submitted.id)) {
        KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
        if (task != nullptr && task->station == "kitchen") {
            kitchenTask = task;
        }
        if (task != nullptr && task->station == "bar") {
            barTask = task;
        }
    }

    OperationResult readyBeforeStart = kitchenTask == nullptr
        ? OperationResult::failure("Missing task")
        : kitchen_fulfillment::completeTask(database, kitchenTask->id, kitchenTask->station, kitchenTask->station);
    OperationResult wrongStation = kitchenTask == nullptr
        ? OperationResult::failure("Missing task")
        : kitchen_fulfillment::startTask(database, kitchenTask->id, "bar", "bar");

    runner.expect(kitchenTask != nullptr && barTask != nullptr, "KF-01/KF-02 accepted order routes food to kitchen and drink to bar");
    runner.expect(!readyBeforeStart.ok && readyBeforeStart.code == "KITCHEN_TASK_INVALID_STATE", "KF-05 pending task cannot be marked ready before start");
    runner.expect(!wrongStation.ok && wrongStation.code == "KITCHEN_TASK_WRONG_STATION", "kitchen task rejects wrong station update");
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
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}, {2, 1}}, "customer", "issue-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    const std::vector<int> taskIds = taskIdsForOrder(database, submitted.id);
    KitchenTaskRecord *issueTask = taskIds.empty() ? nullptr : database.findKitchenTaskById(taskIds.front());
    KitchenTaskRecord *billableTask = taskIds.size() < 2 ? nullptr : database.findKitchenTaskById(taskIds[1]);

    if (billableTask != nullptr) {
        startAndReadyTask(database, billableTask->id);
    }
    if (issueTask != nullptr) {
        kitchen_fulfillment::startTask(database, issueTask->id, issueTask->station, issueTask->station);
    }
    OperationResult issue = issueTask == nullptr
        ? OperationResult::failure("Missing issue task")
        : kitchen_fulfillment::reportIssue(database, issueTask->id, issueTask->station, "ingredient unavailable", issueTask->station);
    OperationResult billBlocked = payment_billing::createBill(database, session->id, "customer");
    OperationResult resolved = kitchen_fulfillment::resolveIssue(database, issue.id, "CANCEL_ITEM", "cashier");
    OperationResult billAllowed = payment_billing::createBill(database, session->id, "customer");
    BillRecord *bill = database.findBillById(billAllowed.id);

    runner.expect(issue.ok, "kitchen can report issue");
    runner.expect(!billBlocked.ok && billBlocked.code == "BILL_BLOCKED_BY_ACTIVE_WORK", "kitchen issue blocks bill");
    runner.expect(resolved.ok, "cashier resolves kitchen issue");
    runner.expect(billAllowed.ok, "bill can be created after issue resolution");
    runner.expect(bill != nullptr && bill->total == 79000 && billLineCount(database, bill->id) == 1, "BP-03 kitchen issue cancel is excluded from bill total");
}

void testBillRequiresOrder(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_bill_requires_order.txt");
    SessionRecord *session = openDemoTable(database, "T02");
    runner.expect(session != nullptr, "open table for bill precondition");

    OperationResult bill = payment_billing::createBill(database, session->id, "customer");

    runner.expect(!bill.ok && bill.code == "BILL_REQUIRES_ORDER", "bill request requires at least one submitted order");
    runner.expect(database.bills.empty(), "bill request before order does not create bill");
}

void testBillWaitsForAllKitchenAndBarTasks(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_bill_waits_all_tasks.txt");
    SessionRecord *session = openDemoTable(database, "T02");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}, {2, 1}, {5, 1}}, "customer", "split-station-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    const std::vector<int> taskIds = taskIdsForOrder(database, submitted.id);

    runner.expect(taskIds.size() == 3, "split kitchen/bar order creates three tasks");
    KitchenTaskRecord *firstTask = taskIds.empty() ? nullptr : database.findKitchenTaskById(taskIds.front());
    runner.expect(firstTask != nullptr, "find first split-station task");
    if (firstTask != nullptr) {
        kitchen_fulfillment::startTask(database, firstTask->id, firstTask->station, firstTask->station);
        kitchen_fulfillment::completeTask(database, firstTask->id, firstTask->station, firstTask->station);
        kitchen_fulfillment::markServed(database, firstTask->id, "waiter");
    }

    OperationResult earlyBill = payment_billing::createBill(database, session->id, "customer");
    database.bills.push_back({database.nextBillId(), session->id, 69000, "OPEN", "cash", 1, session->version, 0, "", "partial-test"});
    OperationResult earlyPayment = payment_billing::confirmPayment(database, database.bills.back().id, "cash", 69000, 1, "partial-pay", "cashier");

    runner.expect(!earlyBill.ok && earlyBill.code == "BILL_BLOCKED_BY_ACTIVE_WORK", "bill is blocked until every kitchen/bar task is ready");
    runner.expect(!earlyPayment.ok && earlyPayment.code == "BILL_BLOCKED_BY_ACTIVE_WORK", "payment is blocked for a partial bill while tasks remain active");

    for (int taskId : taskIds) {
        KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
        if (task == nullptr || task->status == "SERVED") {
            continue;
        }
        if (task->status == "PENDING") {
            kitchen_fulfillment::startTask(database, task->id, task->station, task->station);
        }
        if (task->status == "PREPARING") {
            kitchen_fulfillment::completeTask(database, task->id, task->station, task->station);
        }
    }

    OperationResult finalBill = payment_billing::createBill(database, session->id, "customer");
    BillRecord *bill = database.findBillById(finalBill.id);
    runner.expect(finalBill.ok && bill != nullptr, "bill is created after all split-station tasks are ready");
    runner.expect(bill != nullptr && bill->total == 163000, "bill total includes all kitchen and bar items");
}

void testBillLockAndReopenFlow(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_bill_lock_reopen.txt");
    SessionRecord *session = openDemoTable(database, "T01");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{1, 1}}, "customer", "bill-lock-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *task = firstTaskForOrder(database, submitted.id);
    startAndReadyTask(database, task->id);

    OperationResult firstBill = payment_billing::createBill(database, session->id, "customer");
    OperationResult duplicateBill = payment_billing::createBill(database, session->id, "cashier");
    OperationResult submitWhileBilling = order_management::submitOrder(database, session->id, {{5, 1}}, "customer", "locked-order-key");
    OperationResult reopened = payment_billing::reopenBill(database, firstBill.id, "cashier");
    OperationResult submitAfterReopen = order_management::submitOrder(database, session->id, {{5, 1}}, "customer", "reopened-order-key");
    BillRecord *bill = database.findBillById(firstBill.id);

    runner.expect(firstBill.ok, "BP-01 bill can be created when session is ready");
    runner.expect(duplicateBill.ok && duplicateBill.id == firstBill.id, "BP-04 duplicate bill request returns existing open bill");
    runner.expect(!submitWhileBilling.ok && submitWhileBilling.code == "SESSION_NOT_ACTIVE", "GLOBAL_003 bill-requested session blocks new order");
    runner.expect(reopened.ok && bill != nullptr && bill->status == "VOIDED", "BP-09 reopen voids existing open bill");
    runner.expect(submitAfterReopen.ok, "BP-09 reopened session accepts new orders");
}

void testPaymentSuccessClosesSession(TestRunner &runner) {
    FileDatabase database = freshDatabase("test_payment_success.txt");
    SessionRecord *session = openDemoTable(database, "T03");
    OperationResult submitted = order_management::submitOrder(database, session->id, {{2, 1}}, "customer", "payment-success-key");
    order_management::acceptOrder(database, submitted.id, "cashier");
    KitchenTaskRecord *task = firstTaskForOrder(database, submitted.id);
    startAndReadyTask(database, task->id);
    OperationResult billCreated = payment_billing::createBill(database, session->id, "customer");
    BillRecord *bill = database.findBillById(billCreated.id);
    OperationResult paid = payment_billing::confirmPayment(database, bill->id, "cash", bill->total + 1000, bill->version, "pay-success-key", "cashier");
    TableRecord *table = database.findTableByCode("T03");

    runner.expect(paid.ok, "BP-12 overpaid cash payment is accepted");
    runner.expect(bill != nullptr && bill->status == "PAID" && bill->paidAmount == 80000, "BP-06 payment marks bill paid with received amount");
    runner.expect(session->status == "CLOSED", "BP-06 payment closes dining session");
    runner.expect(table != nullptr && table->state == "CLEANING" && table->activeSessionId == 0, "BP-06 payment moves table to cleaning");
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
    testTableSessionEdges(runner);
    testMergeTablesMovesOrders(runner);
    testSubmitGuards(runner);
    testDoubleSubmit(runner);
    testCancelPendingItemAndBillExcludesIt(runner);
    testCancelPreparingItemIsDenied(runner);
    testKitchenTaskTransitions(runner);
    testSoldOutDecisionAndReplacement(runner);
    testKitchenIssueBlocksAndResolvesBill(runner);
    testBillRequiresOrder(runner);
    testBillWaitsForAllKitchenAndBarTasks(runner);
    testBillLockAndReopenFlow(runner);
    testPaymentSuccessClosesSession(runner);
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
