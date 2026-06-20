#pragma once

#include <string>
#include <utility>
#include <vector>

namespace app {

struct TableRecord {
    int id{};
    std::string code;
    std::string state;
    int activeSessionId{};
};

struct SessionRecord {
    int id{};
    std::string status;
    std::vector<int> tableIds;
    std::string openedAt;
    std::string closedAt;
    int version{};
};

struct MenuItemRecord {
    int id{};
    std::string category;
    std::string name;
    int price{};
    std::string catalogStatus;
    std::string availabilityStatus;
    std::string station;
    int prepMinutes{};
};

struct OrderRecord {
    int id{};
    int sessionId{};
    std::string status;
    std::string note;
    std::string createdAt;
    std::string clientRequestId;
};

struct OrderItemRecord {
    int id{};
    int orderId{};
    int menuItemId{};
    int quantity{};
    int unitPrice{};
    std::string status;
    std::string note;
};

struct KitchenTaskRecord {
    int id{};
    int orderItemId{};
    std::string station;
    std::string status;
    std::string issue;
    int issueId{};
};

struct BillRecord {
    int id{};
    int sessionId{};
    int total{};
    std::string status;
    std::string paymentMethod;
    int version{};
    int sessionVersion{};
    int paidAmount{};
    std::string paidAt;
    std::string idempotencyKey;
};

struct BillLineRecord {
    int id{};
    int billId{};
    int orderItemId{};
    int menuItemId{};
    std::string itemName;
    int quantity{};
    int unitPrice{};
    int lineTotal{};
    std::string sourceStatus;
};

struct PaymentRecord {
    int id{};
    int billId{};
    int paidAmount{};
    std::string method;
    std::string status;
    std::string idempotencyKey;
    std::string paidAt;
};

struct IdempotencyRecord {
    int id{};
    std::string scope;
    std::string key;
    std::string operation;
    int entityId{};
    std::string createdAt;
};

struct KitchenIssueRecord {
    int id{};
    int taskId{};
    int orderItemId{};
    std::string reason;
    std::string resolution;
    std::string status;
    std::string createdAt;
    std::string resolvedAt;
};

struct StaffUserRecord {
    int id{};
    std::string actor;
    std::string role;
    bool active{};
};

struct RolePermissionRecord {
    int id{};
    std::string role;
    std::string permissionKey;
};

struct AuditEventRecord {
    int id{};
    std::string role;
    std::string message;
    std::string createdAt;
    std::string action;
    std::string severity;
    std::string entityType;
    int entityId{};
    std::string correlationId;
};

struct NotificationRecord {
    int id{};
    std::string channel;
    std::string type;
    std::string message;
    std::string entityType;
    int entityId{};
    std::string createdAt;
    std::string correlationId;
    bool read{};
};

struct OperationResult {
    bool ok{};
    std::string message;
    int id{};
    std::string code;
    std::string requiredAction;
    std::string contextJson = "{}";
    std::string correlationId;
    bool auditRequired{};
    std::vector<std::string> notificationTargets;

    static OperationResult success(std::string message, int id = 0, std::string code = "OK") {
        OperationResult result;
        result.ok = true;
        result.message = std::move(message);
        result.id = id;
        result.code = std::move(code);
        return result;
    }

    static OperationResult failure(std::string message, std::string code = "BUSINESS_RULE_DENIED", std::string requiredAction = "") {
        OperationResult result;
        result.ok = false;
        result.message = std::move(message);
        result.code = std::move(code);
        result.requiredAction = std::move(requiredAction);
        result.auditRequired = true;
        return result;
    }
};

}
