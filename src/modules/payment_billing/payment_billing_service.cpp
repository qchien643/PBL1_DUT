#include "payment_billing_service.hpp"

#include "../../policies/business_policies.hpp"
#include "../../shared/utils.hpp"

#include <algorithm>

namespace app::payment_billing {

namespace {

bool isBillableItem(const FileDatabase &database, const OrderItemRecord &item) {
    if (item.status == "CANCELLED" || item.status == "REJECTED" ||
        item.status == "SUBMITTED" || item.status == "NEEDS_CUSTOMER_CONFIRMATION" ||
        item.status == "CANCEL_REQUESTED" || item.status == "ISSUE_PENDING_DECISION") {
        return false;
    }
    const KitchenTaskRecord *task = nullptr;
    for (const KitchenTaskRecord &candidate : database.kitchenTasks) {
        if (candidate.orderItemId == item.id && candidate.status != "CANCELLED") {
            task = &candidate;
        }
    }
    if (task == nullptr) {
        return item.status == "READY" || item.status == "SERVED";
    }
    return task->status == "READY" || task->status == "SERVED";
}

int currentBillVersion(const FileDatabase &database, const BillRecord &bill) {
    int version = bill.version <= 0 ? 1 : bill.version;
    for (const BillRecord &candidate : database.bills) {
        if (candidate.sessionId == bill.sessionId && candidate.id != bill.id) {
            version = std::max(version, candidate.version + 1);
        }
    }
    return version;
}

void rebuildBillLines(FileDatabase &database, BillRecord &bill) {
    database.billLines.erase(
        std::remove_if(database.billLines.begin(), database.billLines.end(), [&](const BillLineRecord &line) {
            return line.billId == bill.id;
        }),
        database.billLines.end());

    int total = 0;
    for (const OrderRecord &order : database.orders) {
        if (order.sessionId != bill.sessionId || order.status == "REJECTED") {
            continue;
        }
        for (const OrderItemRecord &item : database.orderItems) {
            if (item.orderId != order.id || !isBillableItem(database, item)) {
                continue;
            }
            MenuItemRecord *menuItem = database.findMenuItemById(item.menuItemId);
            const int lineTotal = item.quantity * item.unitPrice;
            total += lineTotal;
            database.billLines.push_back({
                database.nextBillLineId(),
                bill.id,
                item.id,
                item.menuItemId,
                menuItem == nullptr ? "Unknown item" : menuItem->name,
                item.quantity,
                item.unitPrice,
                lineTotal,
                item.status,
            });
        }
    }
    bill.total = total;
}

int calculateCurrentTotal(const FileDatabase &database, int sessionId) {
    int total = 0;
    for (const OrderRecord &order : database.orders) {
        if (order.sessionId != sessionId || order.status == "REJECTED") {
            continue;
        }
        for (const OrderItemRecord &item : database.orderItems) {
            if (item.orderId == order.id && isBillableItem(database, item)) {
                total += item.quantity * item.unitPrice;
            }
        }
    }
    return total;
}

}

int billTotal(const FileDatabase &database, int sessionId) {
    return calculateCurrentTotal(database, sessionId);
}

OperationResult createBill(FileDatabase &database, int sessionId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "bill.create");
    if (!permission.ok) {
        return permission;
    }

    SessionRecord *session = database.findSessionById(sessionId);
    if (session == nullptr || session->status == "CLOSED") {
        return OperationResult::failure("No active session for this table.", "SESSION_NOT_ACTIVE", "OPEN_TABLE_FIRST");
    }
    OperationResult billDecision = policy::evaluateCreateBill(database, *session);
    if (!billDecision.ok) {
        return billDecision;
    }

    BillRecord *bill = database.findOpenBillBySessionId(session->id);
    if (bill != nullptr && bill->sessionVersion != session->version) {
        bill->status = "STALE";
        database.addAudit(actor, "MarkBillStale", "Marked bill #" + std::to_string(bill->id) + " stale before recalculation", "HIGH", "bill", bill->id);
        bill = nullptr;
    }

    if (bill == nullptr) {
        BillRecord newBill;
        newBill.id = database.nextBillId();
        newBill.sessionId = session->id;
        newBill.status = "OPEN";
        newBill.version = 1;
        newBill.sessionVersion = session->version;
        database.bills.push_back(newBill);
        bill = &database.bills.back();
    } else if (bill->version <= 0) {
        bill->version = currentBillVersion(database, *bill);
    }

    bill->sessionVersion = session->version;
    rebuildBillLines(database, *bill);
    session->status = "BILL_REQUESTED";
    database.addAudit(actor, "CreateBill", "Created bill #" + std::to_string(bill->id) + " for session #" + std::to_string(session->id), "HIGH", "bill", bill->id);
    database.save();
    return OperationResult::success("Bill #" + std::to_string(bill->id) + " is ready for payment confirmation.", bill->id);
}

OperationResult createBillForTable(FileDatabase &database, const std::string &tableCode, const std::string &actor) {
    SessionRecord *session = database.findActiveSessionByTableCode(tableCode);
    if (session == nullptr) {
        return OperationResult::failure("No active session for this table.");
    }
    return createBill(database, session->id, actor);
}

OperationResult confirmPayment(FileDatabase &database, int billId, const std::string &paymentMethod, const std::string &actor) {
    BillRecord *bill = database.findBillById(billId);
    const int fallbackAmount = bill == nullptr ? 0 : bill->total;
    const int fallbackVersion = bill == nullptr ? 0 : bill->version;
    return confirmPayment(database, billId, paymentMethod, fallbackAmount, fallbackVersion, "", actor);
}

OperationResult confirmPayment(FileDatabase &database, int billId, const std::string &paymentMethod, int paidAmount, int billVersion, const std::string &idempotencyKey, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "bill.pay");
    if (!permission.ok) {
        return permission;
    }

    BillRecord *bill = database.findBillById(billId);
    if (!idempotencyKey.empty()) {
        IdempotencyRecord *existing = database.findIdempotency("bill:" + std::to_string(billId), idempotencyKey, "bill.pay");
        if (existing != nullptr) {
            return OperationResult::success("Duplicate payment ignored. Existing payment returned.", existing->entityId, "IDEMPOTENT_REPLAY");
        }
    }
    if (bill == nullptr) {
        return OperationResult::failure("Bill not found.", "BILL_NOT_FOUND", "RELOAD_BILL");
    }

    SessionRecord *session = database.findSessionById(bill->sessionId);
    if (session != nullptr) {
        const int currentTotal = calculateCurrentTotal(database, session->id);
        if (currentTotal != bill->total || bill->sessionVersion != session->version) {
            bill->status = "STALE";
            database.addAudit(actor, "BillStaleAtPayment", "Payment denied because bill #" + std::to_string(bill->id) + " is stale", "HIGH", "bill", bill->id);
            database.save();
            return OperationResult::failure("Bill is stale. Recalculate before payment.", "BILL_STALE_RECALCULATE_REQUIRED", "RECALCULATE_BILL");
        }
    }

    OperationResult paymentDecision = policy::evaluatePayment(database, *bill, paidAmount, billVersion);
    if (!paymentDecision.ok) {
        return paymentDecision;
    }

    bill->status = "PAID";
    bill->paymentMethod = paymentMethod.empty() ? "cash" : paymentMethod;
    bill->paidAmount = paidAmount;
    bill->paidAt = nowStamp();
    bill->idempotencyKey = idempotencyKey;

    const int paymentId = database.nextPaymentId();
    database.payments.push_back({paymentId, bill->id, paidAmount, bill->paymentMethod, "COMPLETED", idempotencyKey, bill->paidAt});
    if (!idempotencyKey.empty()) {
        database.idempotencyKeys.push_back({database.nextIdempotencyId(), "bill:" + std::to_string(billId), idempotencyKey, "bill.pay", paymentId, nowStamp()});
    }

    if (session != nullptr) {
        session->status = "CLOSED";
        session->closedAt = nowStamp();
        for (int tableId : session->tableIds) {
            TableRecord *table = database.findTableById(tableId);
            if (table != nullptr) {
                table->state = "CLEANING";
                table->activeSessionId = 0;
            }
        }
    }

    database.addAudit(actor, "ConfirmPayment", "Confirmed payment for bill #" + std::to_string(bill->id) + " with " + std::to_string(paidAmount), "HIGH", "bill", bill->id);
    database.save();
    return OperationResult::success("Payment confirmed. Table(s) moved to CLEANING.");
}

OperationResult reopenBill(FileDatabase &database, int billId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "bill.reopen");
    if (!permission.ok) {
        return permission;
    }

    BillRecord *bill = database.findBillById(billId);
    if (bill == nullptr || bill->status != "OPEN") {
        return OperationResult::failure("Only an OPEN bill can be reopened.", "BILL_NOT_OPEN", "RELOAD_BILL");
    }
    SessionRecord *session = database.findSessionById(bill->sessionId);
    if (session == nullptr || session->status == "CLOSED") {
        return OperationResult::failure("Session cannot be reopened.", "SESSION_ALREADY_CLOSED", "CONTACT_MANAGER");
    }

    bill->status = "VOIDED";
    session->status = "ACTIVE";
    database.touchSession(session->id);
    database.addAudit(actor, "ReopenBill", "Voided bill #" + std::to_string(bill->id) + " and reopened session #" + std::to_string(session->id), "HIGH", "bill", bill->id);
    database.save();
    return OperationResult::success("Bill voided. Session is active again.", session->id, "BILL_REOPENED");
}

}
