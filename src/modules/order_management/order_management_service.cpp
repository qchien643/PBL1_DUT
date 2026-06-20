#include "order_management_service.hpp"

#include "../../policies/business_policies.hpp"
#include "../../shared/utils.hpp"

namespace app::order_management {

OperationResult submitOrder(FileDatabase &database, int sessionId, const std::vector<CartLine> &cart, const std::string &actor) {
    return submitOrder(database, sessionId, cart, actor, "");
}

OperationResult submitOrder(FileDatabase &database, int sessionId, const std::vector<CartLine> &cart, const std::string &actor, const std::string &idempotencyKey) {
    OperationResult permission = policy::requirePermission(database, actor, "order.submit");
    if (!permission.ok) {
        return permission;
    }

    SessionRecord *session = database.findSessionById(sessionId);
    if (session == nullptr || session->status != "ACTIVE") {
        return OperationResult::failure("Dining session is not active.", "SESSION_NOT_ACTIVE", "OPEN_TABLE_FIRST");
    }
    if (cart.empty()) {
        return OperationResult::failure("Cart is empty.", "ORDER_CART_EMPTY", "ADD_ITEMS_TO_CART");
    }

    const std::string idempotencyScope = "session:" + std::to_string(sessionId);
    if (!idempotencyKey.empty()) {
        IdempotencyRecord *existing = database.findIdempotency(idempotencyScope, idempotencyKey, "order.submit");
        if (existing != nullptr) {
            return OperationResult::success("Duplicate submit ignored. Existing order returned.", existing->entityId, "IDEMPOTENT_REPLAY");
        }
    }

    int validLineCount = 0;
    for (const CartLine &line : cart) {
        MenuItemRecord *menuItem = database.findMenuItemById(line.first);
        if (line.second > 0 && policy::evaluateOrderableItem(menuItem).ok) {
            ++validLineCount;
        }
    }
    if (validLineCount == 0) {
        return OperationResult::failure("No orderable item in cart.", "ORDER_NO_ORDERABLE_ITEM", "RELOAD_MENU");
    }

    OrderRecord order;
    order.id = database.nextOrderId();
    order.sessionId = session->id;
    order.status = "SUBMITTED";
    order.note = "Submitted from customer CMD";
    order.createdAt = nowStamp();
    order.clientRequestId = idempotencyKey;
    database.orders.push_back(order);

    for (const CartLine &line : cart) {
        MenuItemRecord *menuItem = database.findMenuItemById(line.first);
        if (line.second <= 0 || !policy::evaluateOrderableItem(menuItem).ok) {
            continue;
        }
        database.orderItems.push_back({
            database.nextOrderItemId(),
            order.id,
            menuItem->id,
            line.second,
            menuItem->price,
            "SUBMITTED",
            "",
        });
    }

    if (!idempotencyKey.empty()) {
        database.idempotencyKeys.push_back({database.nextIdempotencyId(), idempotencyScope, idempotencyKey, "order.submit", order.id, nowStamp()});
    }
    database.touchSession(session->id);
    database.addAudit(actor, "SubmitOrder", "Submitted order #" + std::to_string(order.id) + " for session #" + std::to_string(session->id), "MEDIUM", "order", order.id);
    database.save();
    return OperationResult::success("Order submitted. Cashier must approve before kitchen receives it.", order.id);
}

OperationResult acceptOrder(FileDatabase &database, int orderId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "order.accept");
    if (!permission.ok) {
        return permission;
    }

    OrderRecord *order = database.findOrderById(orderId);
    if (order == nullptr || !policy::canApproveOrder(*order)) {
        return OperationResult::failure("Order cannot be approved.", "ORDER_NOT_APPROVABLE", "RELOAD_ORDER");
    }

    std::vector<int> unavailableItemIds;
    for (OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.orderId != order->id || orderItem.status != "SUBMITTED") {
            continue;
        }
        MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
        if (!policy::evaluateOrderableItem(menuItem).ok) {
            unavailableItemIds.push_back(orderItem.id);
        }
    }

    if (!unavailableItemIds.empty()) {
        order->status = "NEEDS_CUSTOMER_CONFIRMATION";
        for (OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId != order->id || orderItem.status != "SUBMITTED") {
                continue;
            }
            for (int unavailableItemId : unavailableItemIds) {
                if (orderItem.id == unavailableItemId) {
                    orderItem.status = "NEEDS_CUSTOMER_CONFIRMATION";
                    orderItem.note = "Sold out when cashier tried to accept";
                }
            }
        }
        database.touchSession(order->sessionId);
        database.addAudit(actor, "OrderNeedsCustomerConfirmation", "Order #" + std::to_string(order->id) + " has sold-out item(s)", "HIGH", "order", order->id);
        database.save();

        OperationResult result = OperationResult::success("Some item(s) are sold out. Customer must confirm before kitchen receives this order.", order->id, "UNAVAILABLE_ITEM_REQUIRES_CUSTOMER_CONFIRMATION");
        result.requiredAction = "ASK_CUSTOMER_TO_MODIFY_ORDER";
        result.auditRequired = true;
        result.contextJson = "{\"orderId\":" + std::to_string(order->id) + ",\"unavailableItemCount\":" + std::to_string(unavailableItemIds.size()) + "}";
        result.notificationTargets = {"customer", "cashier"};
        return result;
    }

    int acceptedCount = 0;
    for (OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.orderId != order->id || orderItem.status != "SUBMITTED") {
            continue;
        }

        MenuItemRecord *menuItem = database.findMenuItemById(orderItem.menuItemId);
        orderItem.status = "ACCEPTED";
        database.kitchenTasks.push_back({
            database.nextKitchenTaskId(),
            orderItem.id,
            menuItem->station,
            "PENDING",
            "",
        });
        ++acceptedCount;
    }

    order->status = acceptedCount > 0 ? "ACCEPTED" : "REJECTED";
    database.touchSession(order->sessionId);
    database.addAudit(actor, "AcceptOrder", "Approved order #" + std::to_string(order->id) + " with " + std::to_string(acceptedCount) + " item(s)", "MEDIUM", "order", order->id);
    database.save();
    return OperationResult::success("Order updated. Accepted items: " + std::to_string(acceptedCount) + ".", order->id);
}

OperationResult rejectOrder(FileDatabase &database, int orderId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "order.reject");
    if (!permission.ok) {
        return permission;
    }

    OrderRecord *order = database.findOrderById(orderId);
    if (order == nullptr || (order->status != "SUBMITTED" && order->status != "NEEDS_CUSTOMER_CONFIRMATION")) {
        return OperationResult::failure("Order cannot be rejected.", "ORDER_NOT_REJECTABLE", "RELOAD_ORDER");
    }

    order->status = "REJECTED";
    for (OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.orderId == order->id) {
            orderItem.status = "REJECTED";
        }
    }
    database.touchSession(order->sessionId);
    database.addAudit(actor, "RejectOrder", "Rejected order #" + std::to_string(order->id), "MEDIUM", "order", order->id);
    database.save();
    return OperationResult::success("Order rejected.");
}

OperationResult resolveCustomerDecision(FileDatabase &database, int orderId, const std::string &decision, const std::vector<CartLine> &replacements, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "order.customer_decision");
    if (!permission.ok) {
        return permission;
    }

    OrderRecord *order = database.findOrderById(orderId);
    if (order == nullptr || order->status != "NEEDS_CUSTOMER_CONFIRMATION") {
        return OperationResult::failure("Order is not waiting for customer confirmation.", "ORDER_DECISION_NOT_REQUIRED", "RELOAD_ORDER");
    }
    if (decision != "CANCEL_ORDER" && decision != "REMOVE_UNAVAILABLE" && decision != "REPLACE_ITEMS") {
        return OperationResult::failure("Unknown customer decision.", "ORDER_DECISION_INVALID", "CHOOSE_VALID_DECISION");
    }
    if (decision == "REPLACE_ITEMS") {
        for (const CartLine &replacement : replacements) {
            MenuItemRecord *menuItem = database.findMenuItemById(replacement.first);
            OperationResult itemDecision = policy::evaluateOrderableItem(menuItem);
            if (!itemDecision.ok || replacement.second <= 0) {
                return OperationResult::failure("Replacement item is not available.", "REPLACEMENT_ITEM_NOT_ORDERABLE", "CHOOSE_ANOTHER_ITEM");
            }
        }
    }

    int rejectedUnavailable = 0;
    for (OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.orderId != order->id || orderItem.status != "NEEDS_CUSTOMER_CONFIRMATION") {
            continue;
        }
        orderItem.status = "REJECTED";
        orderItem.note = "Customer resolved sold-out item";
        ++rejectedUnavailable;
    }

    if (decision == "CANCEL_ORDER") {
        order->status = "REJECTED";
        for (OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId == order->id && orderItem.status == "SUBMITTED") {
                orderItem.status = "REJECTED";
                orderItem.note = "Customer cancelled after sold-out confirmation";
            }
        }
        database.touchSession(order->sessionId);
        database.addAudit(actor, "CustomerCancelOrder", "Customer cancelled order #" + std::to_string(order->id) + " after sold-out confirmation", "HIGH", "order", order->id);
        database.save();
        return OperationResult::success("Order cancelled by customer decision.", order->id, "CUSTOMER_DECISION_APPLIED");
    }

    if (decision == "REPLACE_ITEMS") {
        for (const CartLine &replacement : replacements) {
            MenuItemRecord *menuItem = database.findMenuItemById(replacement.first);
            database.orderItems.push_back({
                database.nextOrderItemId(),
                order->id,
                menuItem->id,
                replacement.second,
                menuItem->price,
                "SUBMITTED",
                "Replacement for sold-out item",
            });
        }
    }

    bool hasRemainingSubmitted = false;
    for (const OrderItemRecord &orderItem : database.orderItems) {
        if (orderItem.orderId == order->id && orderItem.status == "SUBMITTED") {
            hasRemainingSubmitted = true;
            break;
        }
    }
    order->status = hasRemainingSubmitted ? "SUBMITTED" : "REJECTED";
    database.touchSession(order->sessionId);
    database.addAudit(actor, "CustomerResolveSoldOut", "Customer resolved sold-out order #" + std::to_string(order->id) + " with decision " + decision + " and rejected " + std::to_string(rejectedUnavailable) + " item(s)", "HIGH", "order", order->id);
    database.save();
    return OperationResult::success(hasRemainingSubmitted ? "Decision saved. Cashier can accept the updated order." : "No item remains; order rejected.", order->id, "CUSTOMER_DECISION_APPLIED");
}

OperationResult requestCancel(FileDatabase &database, int sessionId, int orderItemId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "order.cancel.request");
    if (!permission.ok) {
        return permission;
    }

    OrderItemRecord *orderItem = database.findOrderItemById(orderItemId);
    if (orderItem == nullptr || !policy::canRequestCancel(*orderItem)) {
        return OperationResult::failure("This item cannot be requested for cancellation.", "ORDER_ITEM_CANCEL_NOT_ALLOWED", "CONTACT_STAFF");
    }

    OrderRecord *order = database.findOrderById(orderItem->orderId);
    if (order == nullptr || order->sessionId != sessionId) {
        return OperationResult::failure("Order item does not belong to this dining session.", "ORDER_ITEM_SESSION_MISMATCH", "RELOAD_ORDER");
    }

    orderItem->status = "CANCEL_REQUESTED";
    database.touchSession(sessionId);
    database.addAudit(actor, "RequestCancelItem", "Requested cancel for order item #" + std::to_string(orderItem->id), "MEDIUM", "order_item", orderItem->id);
    database.save();
    return OperationResult::success("Cancel request sent. Staff will approve if kitchen has not started.");
}

OperationResult approveCancel(FileDatabase &database, int orderItemId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "order.cancel.approve");
    if (!permission.ok) {
        return permission;
    }

    OrderItemRecord *orderItem = database.findOrderItemById(orderItemId);
    if (orderItem == nullptr) {
        return OperationResult::failure("Order item not found.", "ORDER_ITEM_NOT_FOUND", "RELOAD_ORDER");
    }

    KitchenTaskRecord *task = database.findKitchenTaskByOrderItemId(orderItem->id);
    if (!policy::canApproveCancel(*orderItem, task)) {
        return OperationResult::failure("Cannot cancel. Item may already be preparing/ready. Restaurant policy requires staff explanation.", "ORDER_ITEM_CANCEL_NOT_ALLOWED", "CONTACT_STAFF");
    }

    orderItem->status = "CANCELLED";
    if (task != nullptr) {
        task->status = "CANCELLED";
    }

    markOrderCompletedIfReady(database, orderItem->orderId);
    OrderRecord *order = database.findOrderById(orderItem->orderId);
    if (order != nullptr) {
        database.touchSession(order->sessionId);
    }
    database.addAudit(actor, "ApproveCancelItem", "Approved cancel for order item #" + std::to_string(orderItem->id), "HIGH", "order_item", orderItem->id);
    database.save();
    return OperationResult::success("Cancelled. This item will not be charged.");
}

void markOrderCompletedIfReady(FileDatabase &database, int orderId) {
    OrderRecord *order = database.findOrderById(orderId);
    if (order == nullptr || order->status == "REJECTED") {
        return;
    }

    bool hasActiveItem = false;
    bool allDone = true;
    for (const OrderItemRecord &item : database.orderItems) {
        if (item.orderId != orderId || item.status == "REJECTED" || item.status == "CANCELLED") {
            continue;
        }
        hasActiveItem = true;
        if (item.status != "READY" && item.status != "SERVED") {
            allDone = false;
        }
    }

    if (!hasActiveItem) {
        order->status = "REJECTED";
    } else if (allDone) {
        order->status = "COMPLETED";
    }
}

}
