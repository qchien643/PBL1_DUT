#pragma once

#include "../domain/models.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace app {

class FileDatabase {
public:
    explicit FileDatabase(std::string filePath);

    std::vector<TableRecord> tables;
    std::vector<SessionRecord> sessions;
    std::vector<MenuItemRecord> menuItems;
    std::vector<OrderRecord> orders;
    std::vector<OrderItemRecord> orderItems;
    std::vector<KitchenTaskRecord> kitchenTasks;
    std::vector<BillRecord> bills;
    std::vector<BillLineRecord> billLines;
    std::vector<PaymentRecord> payments;
    std::vector<IdempotencyRecord> idempotencyKeys;
    std::vector<KitchenIssueRecord> kitchenIssues;
    std::vector<StaffUserRecord> staffUsers;
    std::vector<RolePermissionRecord> rolePermissions;
    std::vector<AuditEventRecord> auditEvents;
    std::vector<NotificationRecord> notifications;

    void loadOrSeed();
    bool load();
    bool save() const;
    void seed();

    int nextSessionId() const;
    int nextOrderId() const;
    int nextOrderItemId() const;
    int nextKitchenTaskId() const;
    int nextBillId() const;
    int nextBillLineId() const;
    int nextPaymentId() const;
    int nextIdempotencyId() const;
    int nextKitchenIssueId() const;
    int nextStaffUserId() const;
    int nextRolePermissionId() const;
    int nextAuditEventId() const;
    int nextNotificationId() const;

    TableRecord *findTableByCode(const std::string &code);
    TableRecord *findTableById(int id);
    SessionRecord *findSessionById(int id);
    SessionRecord *findActiveSessionByTableCode(const std::string &tableCode);
    MenuItemRecord *findMenuItemById(int id);
    OrderRecord *findOrderById(int id);
    OrderItemRecord *findOrderItemById(int id);
    KitchenTaskRecord *findKitchenTaskById(int id);
    KitchenTaskRecord *findKitchenTaskByOrderItemId(int orderItemId);
    KitchenIssueRecord *findKitchenIssueById(int id);
    BillRecord *findBillById(int id);
    BillRecord *findOpenBillBySessionId(int sessionId);
    IdempotencyRecord *findIdempotency(const std::string &scope, const std::string &key, const std::string &operation);
    const IdempotencyRecord *findIdempotency(const std::string &scope, const std::string &key, const std::string &operation) const;
    std::string roleForActor(const std::string &actor) const;
    bool roleHasPermission(const std::string &role, const std::string &permissionKey) const;
    void touchSession(int sessionId);

    void addAudit(const std::string &role, const std::string &message);
    void addAudit(
        const std::string &role,
        const std::string &action,
        const std::string &message,
        const std::string &severity,
        const std::string &entityType = "",
        int entityId = 0,
        const std::string &correlationId = "");
    void addNotification(
        const std::string &channel,
        const std::string &type,
        const std::string &message,
        const std::string &entityType = "",
        int entityId = 0);
    std::vector<NotificationRecord> notificationsAfter(const std::string &channel, int afterId, int limit) const;

private:
    std::string path;

    template <typename T>
    int nextId(const std::vector<T> &records) const {
        int maxId = 0;
        for (const T &record : records) {
            maxId = std::max(maxId, record.id);
        }
        return maxId + 1;
    }
};

}
