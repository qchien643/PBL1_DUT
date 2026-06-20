#include "business_policies.hpp"

namespace app::policy {

bool canOpenTable(const TableRecord &table) {
    return table.state == "AVAILABLE" && table.activeSessionId == 0;
}

bool canOrderItem(const MenuItemRecord &item) {
    return item.catalogStatus == "ACTIVE" && item.availabilityStatus == "AVAILABLE";
}

bool canApproveOrder(const OrderRecord &order) {
    return order.status == "SUBMITTED";
}

bool canRequestCancel(const OrderItemRecord &item) {
    return item.status == "SUBMITTED" || item.status == "ACCEPTED";
}

bool canApproveCancel(const OrderItemRecord &item, const KitchenTaskRecord *task) {
    if (item.status != "CANCEL_REQUESTED") {
        return false;
    }
    return task == nullptr || task->status == "PENDING";
}

bool canRequestBill(const FileDatabase &database, const SessionRecord &session) {
    return evaluateCreateBill(database, session).ok;
}

OperationResult requirePermission(const FileDatabase &database, const std::string &actor, const std::string &permissionKey) {
    const std::string role = database.roleForActor(actor);
    if (database.roleHasPermission(role, permissionKey)) {
        return OperationResult::success("Permission granted.", 0, "PERMISSION_GRANTED");
    }
    OperationResult result = OperationResult::failure(
        "Actor '" + actor + "' does not have permission '" + permissionKey + "'.",
        "PERMISSION_DENIED",
        "ASK_MANAGER_OR_USE_CORRECT_ROLE");
    result.contextJson = "{\"actor\":\"" + actor + "\",\"role\":\"" + role + "\",\"permission\":\"" + permissionKey + "\"}";
    return result;
}

OperationResult evaluateOrderableItem(const MenuItemRecord *item) {
    if (item == nullptr) {
        return OperationResult::failure("Menu item not found.", "MENU_ITEM_NOT_FOUND", "RELOAD_MENU");
    }
    if (item->catalogStatus != "ACTIVE") {
        return OperationResult::failure("Menu item is not active.", "MENU_ITEM_NOT_ACTIVE", "CHOOSE_ANOTHER_ITEM");
    }
    if (item->availabilityStatus != "AVAILABLE") {
        OperationResult result = OperationResult::failure(
            "Menu item is sold out. Customer must confirm a new decision.",
            "UNAVAILABLE_ITEM_REQUIRES_CUSTOMER_CONFIRMATION",
            "ASK_CUSTOMER_TO_MODIFY_ORDER");
        result.contextJson = "{\"menuItemId\":" + std::to_string(item->id) + ",\"availabilityStatus\":\"" + item->availabilityStatus + "\"}";
        result.notificationTargets = {"customer", "cashier"};
        return result;
    }
    return OperationResult::success("Item is orderable.", item->id, "ITEM_ORDERABLE");
}

OperationResult evaluateCreateBill(const FileDatabase &database, const SessionRecord &session) {
    if (session.status == "CLOSED") {
        return OperationResult::failure("Dining session is already closed.", "SESSION_ALREADY_CLOSED", "RELOAD_SESSION");
    }

    std::string blockers = "[";
    bool hasBlocker = false;
    auto addBlocker = [&](const std::string &type, int id, const std::string &status) {
        if (hasBlocker) {
            blockers += ",";
        }
        hasBlocker = true;
        blockers += "{\"type\":\"" + type + "\",\"id\":" + std::to_string(id) + ",\"status\":\"" + status + "\"}";
    };

    for (const OrderRecord &order : database.orders) {
        if (order.sessionId != session.id) {
            continue;
        }
        if (order.status == "SUBMITTED" || order.status == "NEEDS_CUSTOMER_CONFIRMATION") {
            addBlocker("order", order.id, order.status);
        }

        for (const OrderItemRecord &item : database.orderItems) {
            if (item.orderId != order.id) {
                continue;
            }
            if (item.status == "SUBMITTED" || item.status == "NEEDS_CUSTOMER_CONFIRMATION" ||
                item.status == "CANCEL_REQUESTED" || item.status == "ACCEPTED" ||
                item.status == "PREPARING" || item.status == "ISSUE_PENDING_DECISION") {
                addBlocker("order_item", item.id, item.status);
            }

            const KitchenTaskRecord *task = nullptr;
            for (const KitchenTaskRecord &candidate : database.kitchenTasks) {
                if (candidate.orderItemId == item.id) {
                    task = &candidate;
                    break;
                }
            }
            if (task != nullptr && (task->status == "PENDING" || task->status == "PREPARING" || task->status == "ISSUE")) {
                addBlocker("kitchen_task", task->id, task->status);
            }
        }
    }
    blockers += "]";

    if (hasBlocker) {
        OperationResult result = OperationResult::failure(
            "Cannot create bill yet. Resolve pending orders, kitchen work, cancellation or issue first.",
            "BILL_BLOCKED_BY_ACTIVE_WORK",
            "RESOLVE_ORDER_OR_KITCHEN_WORK");
        result.contextJson = "{\"blockers\":" + blockers + "}";
        result.notificationTargets = {"cashier", "customer"};
        return result;
    }
    return OperationResult::success("Bill can be created.", session.id, "BILL_ALLOWED");
}

OperationResult evaluatePayment(const FileDatabase &database, const BillRecord &bill, int paidAmount, int requestBillVersion) {
    if (bill.status == "PAID") {
        return OperationResult::failure("Bill has already been paid.", "PAYMENT_ALREADY_COMPLETED", "SHOW_PAID_BILL");
    }
    if (bill.status != "OPEN") {
        return OperationResult::failure("Bill is not open for payment.", "BILL_NOT_OPEN", "RELOAD_BILL");
    }
    const SessionRecord *session = nullptr;
    for (const SessionRecord &candidate : database.sessions) {
        if (candidate.id == bill.sessionId) {
            session = &candidate;
            break;
        }
    }
    if (session != nullptr && bill.sessionVersion != session->version) {
        return OperationResult::failure("Bill is stale. Recalculate before payment.", "BILL_STALE_RECALCULATE_REQUIRED", "RECALCULATE_BILL");
    }
    if (requestBillVersion != 0 && requestBillVersion != bill.version) {
        return OperationResult::failure("Bill version changed. Reload before payment.", "BILL_STALE_RECALCULATE_REQUIRED", "RELOAD_BILL");
    }
    if (paidAmount < bill.total) {
        OperationResult result = OperationResult::failure("Paid amount is less than bill total.", "PAYMENT_AMOUNT_INVALID", "ENTER_SUFFICIENT_AMOUNT");
        result.contextJson = "{\"billTotal\":" + std::to_string(bill.total) + ",\"paidAmount\":" + std::to_string(paidAmount) + "}";
        return result;
    }
    return OperationResult::success("Payment can be confirmed.", bill.id, "PAYMENT_ALLOWED");
}

OperationResult evaluateKitchenTaskTransition(const KitchenTaskRecord *task, const std::string &station, const std::string &targetStatus) {
    if (task == nullptr) {
        return OperationResult::failure("Kitchen task not found.", "KITCHEN_TASK_NOT_FOUND", "RELOAD_TASKS");
    }
    if (task->station != station) {
        return OperationResult::failure("Task belongs to another station.", "KITCHEN_TASK_WRONG_STATION", "OPEN_CORRECT_STATION");
    }
    if (targetStatus == "PREPARING" && task->status == "PENDING") {
        return OperationResult::success("Task can start.", task->id, "TASK_START_ALLOWED");
    }
    if (targetStatus == "READY" && (task->status == "PREPARING" || task->status == "READY")) {
        return OperationResult::success("Task can be marked ready.", task->id, "TASK_READY_ALLOWED");
    }
    if (targetStatus == "SERVED" && (task->status == "READY" || task->status == "SERVED")) {
        return OperationResult::success("Task can be served.", task->id, "TASK_SERVED_ALLOWED");
    }
    if (targetStatus == "ISSUE" && (task->status == "PENDING" || task->status == "PREPARING" || task->status == "READY")) {
        return OperationResult::success("Kitchen issue can be reported.", task->id, "TASK_ISSUE_ALLOWED");
    }
    return OperationResult::failure("Kitchen task transition is not allowed.", "KITCHEN_TASK_INVALID_STATE", "RELOAD_TASKS");
}

}
