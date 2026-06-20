#include "kitchen_fulfillment_service.hpp"

#include "../order_management/order_management_service.hpp"
#include "../../policies/business_policies.hpp"
#include "../../shared/utils.hpp"

namespace app::kitchen_fulfillment {

std::vector<KitchenTaskRecord> activeTasksForStation(const FileDatabase &database, const std::string &station) {
    std::vector<KitchenTaskRecord> tasks;
    for (const KitchenTaskRecord &task : database.kitchenTasks) {
        if (task.station == station && task.status != "CANCELLED" && task.status != "SERVED") {
            tasks.push_back(task);
        }
    }
    return tasks;
}

OperationResult startTask(FileDatabase &database, int taskId, const std::string &station, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "kitchen.start");
    if (!permission.ok) {
        return permission;
    }

    KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
    OperationResult transition = policy::evaluateKitchenTaskTransition(task, station, "PREPARING");
    if (!transition.ok) {
        return transition;
    }

    task->status = "PREPARING";
    OrderItemRecord *orderItem = database.findOrderItemById(task->orderItemId);
    if (orderItem != nullptr) {
        orderItem->status = "PREPARING";
        OrderRecord *order = database.findOrderById(orderItem->orderId);
        if (order != nullptr) {
            database.touchSession(order->sessionId);
        }
    }
    database.addAudit(actor, "StartKitchenTask", "Started task #" + std::to_string(task->id), "LOW", "kitchen_task", task->id);
    database.save();
    return OperationResult::success("Task started.");
}

OperationResult completeTask(FileDatabase &database, int taskId, const std::string &station, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "kitchen.ready");
    if (!permission.ok) {
        return permission;
    }

    KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
    OperationResult transition = policy::evaluateKitchenTaskTransition(task, station, "READY");
    if (!transition.ok) {
        return transition;
    }

    if (task->status == "READY") {
        return OperationResult::success("Task is already READY.", task->id, "IDEMPOTENT_REPLAY");
    }

    task->status = "READY";
    OrderItemRecord *orderItem = database.findOrderItemById(task->orderItemId);
    if (orderItem != nullptr) {
        orderItem->status = "READY";
        order_management::markOrderCompletedIfReady(database, orderItem->orderId);
        OrderRecord *order = database.findOrderById(orderItem->orderId);
        if (order != nullptr) {
            database.touchSession(order->sessionId);
        }
    }
    database.addAudit(actor, "CompleteKitchenTask", "Completed task #" + std::to_string(task->id), "LOW", "kitchen_task", task->id);
    database.save();
    return OperationResult::success("Task is READY for serving.");
}

OperationResult markServed(FileDatabase &database, int taskId, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "kitchen.served");
    if (!permission.ok) {
        return permission;
    }

    KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
    if (task == nullptr) {
        return OperationResult::failure("Kitchen task not found.", "KITCHEN_TASK_NOT_FOUND", "RELOAD_TASKS");
    }
    OperationResult transition = policy::evaluateKitchenTaskTransition(task, task->station, "SERVED");
    if (!transition.ok) {
        return transition;
    }
    if (task->status == "SERVED") {
        return OperationResult::success("Task is already served.", task->id, "IDEMPOTENT_REPLAY");
    }

    task->status = "SERVED";
    OrderItemRecord *orderItem = database.findOrderItemById(task->orderItemId);
    if (orderItem != nullptr) {
        orderItem->status = "SERVED";
        order_management::markOrderCompletedIfReady(database, orderItem->orderId);
        OrderRecord *order = database.findOrderById(orderItem->orderId);
        if (order != nullptr) {
            database.touchSession(order->sessionId);
        }
    }
    database.addAudit(actor, "MarkTaskServed", "Marked task #" + std::to_string(task->id) + " served", "MEDIUM", "kitchen_task", task->id);
    database.save();
    return OperationResult::success("Task marked SERVED.");
}

OperationResult reportIssue(FileDatabase &database, int taskId, const std::string &station, const std::string &reason, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "kitchen.issue.report");
    if (!permission.ok) {
        return permission;
    }

    KitchenTaskRecord *task = database.findKitchenTaskById(taskId);
    OperationResult transition = policy::evaluateKitchenTaskTransition(task, station, "ISSUE");
    if (!transition.ok) {
        return transition;
    }
    if (reason.empty()) {
        return OperationResult::failure("Kitchen issue reason is required.", "KITCHEN_ISSUE_REASON_REQUIRED", "ENTER_REASON");
    }

    OrderItemRecord *orderItem = database.findOrderItemById(task->orderItemId);
    const int issueId = database.nextKitchenIssueId();
    database.kitchenIssues.push_back({issueId, task->id, task->orderItemId, reason, "", "OPEN", nowStamp(), ""});
    task->status = "ISSUE";
    task->issue = reason;
    task->issueId = issueId;
    if (orderItem != nullptr) {
        orderItem->status = "ISSUE_PENDING_DECISION";
        orderItem->note = reason;
        OrderRecord *order = database.findOrderById(orderItem->orderId);
        if (order != nullptr) {
            database.touchSession(order->sessionId);
        }
    }

    database.addAudit(actor, "ReportKitchenIssue", "Reported issue for task #" + std::to_string(task->id) + ": " + reason, "HIGH", "kitchen_issue", issueId);
    database.save();
    return OperationResult::success("Kitchen issue reported. Bill is blocked until staff resolves it.", issueId, "KITCHEN_ISSUE_REPORTED");
}

OperationResult resolveIssue(FileDatabase &database, int issueId, const std::string &resolution, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "kitchen.issue.resolve");
    if (!permission.ok) {
        return permission;
    }

    KitchenIssueRecord *issue = database.findKitchenIssueById(issueId);
    if (issue == nullptr || issue->status != "OPEN") {
        return OperationResult::failure("Kitchen issue is not open.", "KITCHEN_ISSUE_NOT_OPEN", "RELOAD_ISSUES");
    }

    KitchenTaskRecord *task = database.findKitchenTaskById(issue->taskId);
    OrderItemRecord *orderItem = database.findOrderItemById(issue->orderItemId);
    if (task == nullptr || orderItem == nullptr) {
        return OperationResult::failure("Kitchen issue data is incomplete.", "KITCHEN_ISSUE_INVALID_STATE", "CONTACT_MANAGER");
    }

    if (resolution == "CANCEL_ITEM") {
        task->status = "CANCELLED";
        orderItem->status = "CANCELLED";
        orderItem->note = "Cancelled after kitchen issue: " + issue->reason;
    } else if (resolution == "REMAKE_SAME_ITEM") {
        task->status = "CANCELLED";
        orderItem->status = "ACCEPTED";
        orderItem->note = "Remake after kitchen issue: " + issue->reason;
        database.kitchenTasks.push_back({database.nextKitchenTaskId(), orderItem->id, task->station, "PENDING", "", 0});
    } else if (resolution == "KEEP_AND_BILL_WITH_MANAGER_OVERRIDE") {
        task->status = "READY";
        orderItem->status = "READY";
        orderItem->note = "Manager override after issue: " + issue->reason;
    } else {
        return OperationResult::failure("Unknown kitchen issue resolution.", "KITCHEN_ISSUE_RESOLUTION_INVALID", "CHOOSE_VALID_RESOLUTION");
    }

    issue->status = "RESOLVED";
    issue->resolution = resolution;
    issue->resolvedAt = nowStamp();
    OrderRecord *order = database.findOrderById(orderItem->orderId);
    if (order != nullptr) {
        database.touchSession(order->sessionId);
        order_management::markOrderCompletedIfReady(database, order->id);
    }
    database.addAudit(actor, "ResolveKitchenIssue", "Resolved kitchen issue #" + std::to_string(issueId) + " with " + resolution, "HIGH", "kitchen_issue", issueId);
    database.save();
    return OperationResult::success("Kitchen issue resolved.", issueId, "KITCHEN_ISSUE_RESOLVED");
}

}
