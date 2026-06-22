#include "file_database.hpp"

#include "storage/file_table_store.hpp"
#include "../shared/utils.hpp"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace app {

namespace {

std::filesystem::path databaseRootForPath(const std::string &path) {
    const std::filesystem::path inputPath(path);
    if (inputPath.has_extension() && inputPath.extension() == ".txt") {
        if (inputPath.stem() == "restaurant_db") {
            return inputPath.parent_path() / "db";
        }
        return inputPath.parent_path() / (inputPath.stem().string() + "_db");
    }
    return inputPath;
}

std::string field(const storage::Row &row, size_t index, const std::string &fallback = "") {
    return index < row.size() ? row[index] : fallback;
}

int intField(const storage::Row &row, size_t index, int fallback = 0) {
    try {
        return std::stoi(field(row, index, std::to_string(fallback)));
    } catch (...) {
        return fallback;
    }
}

void clearRecords(FileDatabase &database) {
    database.tables.clear();
    database.sessions.clear();
    database.menuItems.clear();
    database.orders.clear();
    database.orderItems.clear();
    database.kitchenTasks.clear();
    database.bills.clear();
    database.billLines.clear();
    database.payments.clear();
    database.idempotencyKeys.clear();
    database.kitchenIssues.clear();
    database.staffUsers.clear();
    database.rolePermissions.clear();
    database.auditEvents.clear();
    database.notifications.clear();
}

void ensureDefaultStaffAndPermissions(FileDatabase &database) {
    if (database.staffUsers.empty()) {
        database.staffUsers = {
            {1, "manager", "manager", true},
            {2, "cashier", "cashier", true},
            {3, "waiter", "waiter", true},
            {4, "kitchen", "kitchen", true},
            {5, "bar", "kitchen", true},
            {6, "customer", "customer", true},
        };
    }
    if (database.rolePermissions.empty()) {
        database.rolePermissions = {
            {1, "manager", "*"},
            {2, "cashier", "table.open"},
            {3, "cashier", "table.merge"},
            {4, "cashier", "table.transfer"},
            {5, "cashier", "table.cleaned"},
            {6, "cashier", "order.accept"},
            {7, "cashier", "order.reject"},
            {8, "cashier", "order.cancel.approve"},
            {9, "cashier", "bill.create"},
            {10, "cashier", "bill.pay"},
            {11, "cashier", "bill.reopen"},
            {12, "cashier", "kitchen.issue.resolve"},
            {13, "waiter", "kitchen.served"},
            {14, "kitchen", "kitchen.start"},
            {15, "kitchen", "kitchen.ready"},
            {16, "kitchen", "kitchen.issue.report"},
            {17, "customer", "order.submit"},
            {18, "customer", "order.cancel.request"},
            {19, "customer", "order.customer_decision"},
            {20, "customer", "bill.create"},
            {21, "manager", "menu.availability"},
        };
    }
}

std::string boolText(bool value) {
    return value ? "1" : "0";
}

std::string intText(int value) {
    return std::to_string(value);
}

}

FileDatabase::FileDatabase(std::string filePath) : path(std::move(filePath)) {}

void FileDatabase::loadOrSeed() {
    if (!load()) {
        seed();
        save();
    }
}

bool FileDatabase::load() {
    const storage::FileTableStore store(databaseRootForPath(path));
    if (!store.tableExists("core/dining_tables")) {
        return false;
    }

    clearRecords(*this);

    for (const storage::Row &row : store.loadRows("core/dining_tables")) {
        if (row.size() >= 4) {
            tables.push_back({intField(row, 0), field(row, 1), field(row, 2), intField(row, 3)});
        }
    }

    for (const storage::Row &row : store.loadRows("core/dining_sessions")) {
        if (row.size() >= 5) {
            sessions.push_back({intField(row, 0), field(row, 1), parseIds(field(row, 2)), restore(field(row, 3)), restore(field(row, 4)), intField(row, 5)});
        }
    }

    for (const storage::Row &row : store.loadRows("menu/menu_items")) {
        if (row.size() >= 8) {
            menuItems.push_back({intField(row, 0), field(row, 1), restore(field(row, 2)), intField(row, 3), field(row, 4), field(row, 5), field(row, 6), intField(row, 7)});
        }
    }

    for (const storage::Row &row : store.loadRows("orders/order_headers")) {
        if (row.size() >= 5) {
            orders.push_back({intField(row, 0), intField(row, 1), field(row, 2), restore(field(row, 3)), restore(field(row, 4)), restore(field(row, 5))});
        }
    }

    for (const storage::Row &row : store.loadRows("orders/order_items")) {
        if (row.size() >= 7) {
            orderItems.push_back({intField(row, 0), intField(row, 1), intField(row, 2), intField(row, 3), intField(row, 4), field(row, 5), restore(field(row, 6))});
        }
    }

    for (const storage::Row &row : store.loadRows("kitchen/preparation_tasks")) {
        if (row.size() >= 5) {
            kitchenTasks.push_back({intField(row, 0), intField(row, 1), field(row, 2), field(row, 3), restore(field(row, 4)), intField(row, 5)});
        }
    }

    for (const storage::Row &row : store.loadRows("billing/bills")) {
        if (row.size() >= 5) {
            bills.push_back({
                intField(row, 0),
                intField(row, 1),
                intField(row, 2),
                field(row, 3),
                restore(field(row, 4)),
                intField(row, 5, 1),
                intField(row, 6),
                intField(row, 7),
                restore(field(row, 8)),
                restore(field(row, 9)),
            });
        }
    }

    for (const storage::Row &row : store.loadRows("billing/bill_lines")) {
        if (row.size() >= 9) {
            billLines.push_back({intField(row, 0), intField(row, 1), intField(row, 2), intField(row, 3), restore(field(row, 4)), intField(row, 5), intField(row, 6), intField(row, 7), field(row, 8)});
        }
    }

    for (const storage::Row &row : store.loadRows("billing/payments")) {
        if (row.size() >= 7) {
            payments.push_back({intField(row, 0), intField(row, 1), intField(row, 2), field(row, 3), field(row, 4), restore(field(row, 5)), restore(field(row, 6))});
        }
    }

    for (const storage::Row &row : store.loadRows("orders/idempotency_keys")) {
        if (row.size() >= 6) {
            idempotencyKeys.push_back({intField(row, 0), field(row, 1), restore(field(row, 2)), field(row, 3), intField(row, 4), restore(field(row, 5))});
        }
    }

    for (const storage::Row &row : store.loadRows("kitchen/kitchen_issues")) {
        if (row.size() >= 8) {
            kitchenIssues.push_back({intField(row, 0), intField(row, 1), intField(row, 2), restore(field(row, 3)), restore(field(row, 4)), field(row, 5), restore(field(row, 6)), restore(field(row, 7))});
        }
    }

    for (const storage::Row &row : store.loadRows("staff/staff_users")) {
        if (row.size() >= 4) {
            staffUsers.push_back({intField(row, 0), field(row, 1), field(row, 2), field(row, 3) == "1"});
        }
    }

    for (const storage::Row &row : store.loadRows("staff/role_permissions")) {
        if (row.size() >= 3) {
            rolePermissions.push_back({intField(row, 0), field(row, 1), field(row, 2)});
        }
    }

    for (const storage::Row &row : store.loadRows("governance/audit_events")) {
        if (row.size() >= 4) {
            auditEvents.push_back({
                intField(row, 0),
                field(row, 1),
                restore(field(row, 2)),
                restore(field(row, 3)),
                field(row, 4),
                field(row, 5, "LOW"),
                restore(field(row, 6)),
                intField(row, 7),
                restore(field(row, 8)),
            });
        }
    }

    for (const storage::Row &row : store.loadRows("governance/notifications")) {
        if (row.size() >= 7) {
            notifications.push_back({
                intField(row, 0),
                field(row, 1),
                field(row, 2),
                restore(field(row, 3)),
                restore(field(row, 4)),
                intField(row, 5),
                restore(field(row, 6)),
                restore(field(row, 7)),
                field(row, 8) == "1",
            });
        }
    }

    ensureDefaultStaffAndPermissions(*this);
    return true;
}

bool FileDatabase::save() const {
    const storage::FileTableStore store(databaseRootForPath(path));
    bool saved = true;

    saved = store.saveRows("_meta/schema_version", {{"2"}}, {"version"}) && saved;

    std::vector<storage::Row> tableRows;
    for (const TableRecord &table : tables) {
        tableRows.push_back({intText(table.id), table.code, table.state, intText(table.activeSessionId)});
    }
    saved = store.saveRows("core/dining_tables", tableRows, {"id", "code", "state", "activeSessionId"}) && saved;

    std::vector<storage::Row> sessionRows;
    for (const SessionRecord &session : sessions) {
        sessionRows.push_back({intText(session.id), session.status, joinIds(session.tableIds), session.openedAt, session.closedAt, intText(session.version)});
    }
    saved = store.saveRows("core/dining_sessions", sessionRows, {"id", "status", "tableIds", "openedAt", "closedAt", "version"}) && saved;

    std::vector<storage::Row> menuRows;
    for (const MenuItemRecord &item : menuItems) {
        menuRows.push_back({intText(item.id), item.category, item.name, intText(item.price), item.catalogStatus, item.availabilityStatus, item.station, intText(item.prepMinutes)});
    }
    saved = store.saveRows("menu/menu_items", menuRows, {"id", "category", "name", "price", "catalogStatus", "availabilityStatus", "station", "prepMinutes"}) && saved;

    std::vector<storage::Row> orderRows;
    for (const OrderRecord &order : orders) {
        orderRows.push_back({intText(order.id), intText(order.sessionId), order.status, order.note, order.createdAt, order.clientRequestId});
    }
    saved = store.saveRows("orders/order_headers", orderRows, {"id", "sessionId", "status", "note", "createdAt", "clientRequestId"}) && saved;

    std::vector<storage::Row> orderItemRows;
    for (const OrderItemRecord &item : orderItems) {
        orderItemRows.push_back({intText(item.id), intText(item.orderId), intText(item.menuItemId), intText(item.quantity), intText(item.unitPrice), item.status, item.note});
    }
    saved = store.saveRows("orders/order_items", orderItemRows, {"id", "orderId", "menuItemId", "quantity", "unitPrice", "status", "note"}) && saved;

    std::vector<storage::Row> taskRows;
    for (const KitchenTaskRecord &task : kitchenTasks) {
        taskRows.push_back({intText(task.id), intText(task.orderItemId), task.station, task.status, task.issue, intText(task.issueId)});
    }
    saved = store.saveRows("kitchen/preparation_tasks", taskRows, {"id", "orderItemId", "station", "status", "issue", "issueId"}) && saved;

    std::vector<storage::Row> billRows;
    for (const BillRecord &bill : bills) {
        billRows.push_back({intText(bill.id), intText(bill.sessionId), intText(bill.total), bill.status, bill.paymentMethod, intText(bill.version), intText(bill.sessionVersion), intText(bill.paidAmount), bill.paidAt, bill.idempotencyKey});
    }
    saved = store.saveRows("billing/bills", billRows, {"id", "sessionId", "total", "status", "paymentMethod", "version", "sessionVersion", "paidAmount", "paidAt", "idempotencyKey"}) && saved;

    std::vector<storage::Row> billLineRows;
    for (const BillLineRecord &line : billLines) {
        billLineRows.push_back({intText(line.id), intText(line.billId), intText(line.orderItemId), intText(line.menuItemId), line.itemName, intText(line.quantity), intText(line.unitPrice), intText(line.lineTotal), line.sourceStatus});
    }
    saved = store.saveRows("billing/bill_lines", billLineRows, {"id", "billId", "orderItemId", "menuItemId", "itemName", "quantity", "unitPrice", "lineTotal", "sourceStatus"}) && saved;

    std::vector<storage::Row> paymentRows;
    for (const PaymentRecord &payment : payments) {
        paymentRows.push_back({intText(payment.id), intText(payment.billId), intText(payment.paidAmount), payment.method, payment.status, payment.idempotencyKey, payment.paidAt});
    }
    saved = store.saveRows("billing/payments", paymentRows, {"id", "billId", "paidAmount", "method", "status", "idempotencyKey", "paidAt"}) && saved;

    std::vector<storage::Row> idempotencyRows;
    for (const IdempotencyRecord &record : idempotencyKeys) {
        idempotencyRows.push_back({intText(record.id), record.scope, record.key, record.operation, intText(record.entityId), record.createdAt});
    }
    saved = store.saveRows("orders/idempotency_keys", idempotencyRows, {"id", "scope", "key", "operation", "entityId", "createdAt"}) && saved;

    std::vector<storage::Row> issueRows;
    for (const KitchenIssueRecord &issue : kitchenIssues) {
        issueRows.push_back({intText(issue.id), intText(issue.taskId), intText(issue.orderItemId), issue.reason, issue.resolution, issue.status, issue.createdAt, issue.resolvedAt});
    }
    saved = store.saveRows("kitchen/kitchen_issues", issueRows, {"id", "taskId", "orderItemId", "reason", "resolution", "status", "createdAt", "resolvedAt"}) && saved;

    std::vector<storage::Row> staffRows;
    for (const StaffUserRecord &staff : staffUsers) {
        staffRows.push_back({intText(staff.id), staff.actor, staff.role, boolText(staff.active)});
    }
    saved = store.saveRows("staff/staff_users", staffRows, {"id", "actor", "role", "active"}) && saved;

    std::vector<storage::Row> permissionRows;
    for (const RolePermissionRecord &permission : rolePermissions) {
        permissionRows.push_back({intText(permission.id), permission.role, permission.permissionKey});
    }
    saved = store.saveRows("staff/role_permissions", permissionRows, {"id", "role", "permissionKey"}) && saved;

    std::vector<storage::Row> auditRows;
    for (const AuditEventRecord &event : auditEvents) {
        auditRows.push_back({intText(event.id), event.role, event.message, event.createdAt, event.action, event.severity, event.entityType, intText(event.entityId), event.correlationId});
    }
    saved = store.saveRows("governance/audit_events", auditRows, {"id", "role", "message", "createdAt", "action", "severity", "entityType", "entityId", "correlationId"}) && saved;

    std::vector<storage::Row> notificationRows;
    for (const NotificationRecord &notification : notifications) {
        notificationRows.push_back({intText(notification.id), notification.channel, notification.type, notification.message, notification.entityType, intText(notification.entityId), notification.createdAt, notification.correlationId, boolText(notification.read)});
    }
    saved = store.saveRows("governance/notifications", notificationRows, {"id", "channel", "type", "message", "entityType", "entityId", "createdAt", "correlationId", "read"}) && saved;

    if (saved) {
        const std::filesystem::path legacyPath(path);
        if (legacyPath.has_extension() && legacyPath.extension() == ".txt") {
            std::error_code ignoredError;
            std::filesystem::remove(legacyPath, ignoredError);
        }
    }

    return saved;
}

void FileDatabase::seed() {
    tables = {
        {1, "T01", "AVAILABLE", 0},
        {2, "T02", "AVAILABLE", 0},
        {3, "T03", "AVAILABLE", 0},
        {4, "T04", "AVAILABLE", 0},
        {5, "T05", "AVAILABLE", 0},
        {6, "T06", "AVAILABLE", 0},
    };

    sessions.clear();
    orders.clear();
    orderItems.clear();
    kitchenTasks.clear();
    bills.clear();
    billLines.clear();
    payments.clear();
    idempotencyKeys.clear();
    kitchenIssues.clear();
    staffUsers.clear();
    rolePermissions.clear();
    auditEvents.clear();
    notifications.clear();

    menuItems = {
        {1, "food", "Grilled Chicken Rice", 69000, "ACTIVE", "AVAILABLE", "kitchen", 12},
        {2, "food", "Beef Noodle Bowl", 79000, "ACTIVE", "AVAILABLE", "kitchen", 15},
        {3, "food", "Seafood Fried Rice", 89000, "ACTIVE", "AVAILABLE", "kitchen", 14},
        {4, "food", "Vegetable Spring Rolls", 49000, "ACTIVE", "AVAILABLE", "kitchen", 9},
        {5, "drink", "Iced Tea", 15000, "ACTIVE", "AVAILABLE", "bar", 2},
        {6, "drink", "Orange Juice", 35000, "ACTIVE", "AVAILABLE", "bar", 3},
        {7, "dessert", "Caramel Flan", 29000, "ACTIVE", "AVAILABLE", "kitchen", 4},
        {8, "dessert", "Fruit Yogurt", 39000, "ACTIVE", "AVAILABLE", "kitchen", 5},
    };

    staffUsers = {
        {1, "manager", "manager", true},
        {2, "cashier", "cashier", true},
        {3, "waiter", "waiter", true},
        {4, "kitchen", "kitchen", true},
        {5, "bar", "kitchen", true},
        {6, "customer", "customer", true},
    };

    rolePermissions = {
        {1, "manager", "*"},
        {2, "cashier", "table.open"},
        {3, "cashier", "table.merge"},
        {4, "cashier", "table.transfer"},
        {5, "cashier", "table.cleaned"},
        {6, "cashier", "order.accept"},
        {7, "cashier", "order.reject"},
        {8, "cashier", "order.cancel.approve"},
        {9, "cashier", "bill.create"},
        {10, "cashier", "bill.pay"},
        {11, "cashier", "bill.reopen"},
        {12, "cashier", "kitchen.issue.resolve"},
        {13, "waiter", "kitchen.served"},
        {14, "kitchen", "kitchen.start"},
        {15, "kitchen", "kitchen.ready"},
        {16, "kitchen", "kitchen.issue.report"},
        {17, "customer", "order.submit"},
        {18, "customer", "order.cancel.request"},
        {19, "customer", "order.customer_decision"},
        {20, "customer", "bill.create"},
        {21, "manager", "menu.availability"},
    };

    addAudit("system", "Seeded demo data");
}

int FileDatabase::nextSessionId() const { return nextId(sessions); }
int FileDatabase::nextOrderId() const { return nextId(orders); }
int FileDatabase::nextOrderItemId() const { return nextId(orderItems); }
int FileDatabase::nextKitchenTaskId() const { return nextId(kitchenTasks); }
int FileDatabase::nextBillId() const { return nextId(bills); }
int FileDatabase::nextBillLineId() const { return nextId(billLines); }
int FileDatabase::nextPaymentId() const { return nextId(payments); }
int FileDatabase::nextIdempotencyId() const { return nextId(idempotencyKeys); }
int FileDatabase::nextKitchenIssueId() const { return nextId(kitchenIssues); }
int FileDatabase::nextStaffUserId() const { return nextId(staffUsers); }
int FileDatabase::nextRolePermissionId() const { return nextId(rolePermissions); }
int FileDatabase::nextAuditEventId() const { return nextId(auditEvents); }
int FileDatabase::nextNotificationId() const { return nextId(notifications); }

TableRecord *FileDatabase::findTableByCode(const std::string &code) {
    for (TableRecord &table : tables) {
        if (toLower(table.code) == toLower(code)) {
            return &table;
        }
    }
    return nullptr;
}

TableRecord *FileDatabase::findTableById(int id) {
    for (TableRecord &table : tables) {
        if (table.id == id) {
            return &table;
        }
    }
    return nullptr;
}

SessionRecord *FileDatabase::findSessionById(int id) {
    for (SessionRecord &session : sessions) {
        if (session.id == id) {
            return &session;
        }
    }
    return nullptr;
}

SessionRecord *FileDatabase::findActiveSessionByTableCode(const std::string &tableCode) {
    TableRecord *table = findTableByCode(tableCode);
    if (table == nullptr || table->activeSessionId == 0) {
        return nullptr;
    }

    SessionRecord *session = findSessionById(table->activeSessionId);
    if (session == nullptr || session->status == "CLOSED") {
        return nullptr;
    }
    return session;
}

MenuItemRecord *FileDatabase::findMenuItemById(int id) {
    for (MenuItemRecord &item : menuItems) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

OrderRecord *FileDatabase::findOrderById(int id) {
    for (OrderRecord &order : orders) {
        if (order.id == id) {
            return &order;
        }
    }
    return nullptr;
}

OrderItemRecord *FileDatabase::findOrderItemById(int id) {
    for (OrderItemRecord &item : orderItems) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

KitchenTaskRecord *FileDatabase::findKitchenTaskById(int id) {
    for (KitchenTaskRecord &task : kitchenTasks) {
        if (task.id == id) {
            return &task;
        }
    }
    return nullptr;
}

KitchenTaskRecord *FileDatabase::findKitchenTaskByOrderItemId(int orderItemId) {
    for (KitchenTaskRecord &task : kitchenTasks) {
        if (task.orderItemId == orderItemId) {
            return &task;
        }
    }
    return nullptr;
}

KitchenIssueRecord *FileDatabase::findKitchenIssueById(int id) {
    for (KitchenIssueRecord &issue : kitchenIssues) {
        if (issue.id == id) {
            return &issue;
        }
    }
    return nullptr;
}

BillRecord *FileDatabase::findBillById(int id) {
    for (BillRecord &bill : bills) {
        if (bill.id == id) {
            return &bill;
        }
    }
    return nullptr;
}

BillRecord *FileDatabase::findOpenBillBySessionId(int sessionId) {
    for (BillRecord &bill : bills) {
        if (bill.sessionId == sessionId && bill.status == "OPEN") {
            return &bill;
        }
    }
    return nullptr;
}

IdempotencyRecord *FileDatabase::findIdempotency(const std::string &scope, const std::string &key, const std::string &operation) {
    for (IdempotencyRecord &record : idempotencyKeys) {
        if (record.scope == scope && record.key == key && record.operation == operation) {
            return &record;
        }
    }
    return nullptr;
}

const IdempotencyRecord *FileDatabase::findIdempotency(const std::string &scope, const std::string &key, const std::string &operation) const {
    for (const IdempotencyRecord &record : idempotencyKeys) {
        if (record.scope == scope && record.key == key && record.operation == operation) {
            return &record;
        }
    }
    return nullptr;
}

std::string FileDatabase::roleForActor(const std::string &actor) const {
    const std::string normalizedActor = toLower(actor.empty() ? "customer" : actor);
    for (const StaffUserRecord &staff : staffUsers) {
        if (toLower(staff.actor) == normalizedActor && staff.active) {
            return staff.role;
        }
    }
    if (normalizedActor == "bar") {
        return "kitchen";
    }
    if (normalizedActor == "manager" || normalizedActor == "cashier" || normalizedActor == "waiter" || normalizedActor == "kitchen" || normalizedActor == "customer") {
        return normalizedActor;
    }
    return normalizedActor;
}

bool FileDatabase::roleHasPermission(const std::string &role, const std::string &permissionKey) const {
    for (const RolePermissionRecord &permission : rolePermissions) {
        if (permission.role == role && (permission.permissionKey == permissionKey || permission.permissionKey == "*")) {
            return true;
        }
    }
    return false;
}

void FileDatabase::touchSession(int sessionId) {
    SessionRecord *session = findSessionById(sessionId);
    if (session != nullptr) {
        ++session->version;
    }
    for (BillRecord &bill : bills) {
        if (bill.sessionId == sessionId && bill.status == "OPEN") {
            bill.status = "STALE";
        }
    }
}

void FileDatabase::addAudit(const std::string &role, const std::string &message) {
    addAudit(role, "GENERIC", message, "LOW");
}

void FileDatabase::addAudit(
    const std::string &role,
    const std::string &action,
    const std::string &message,
    const std::string &severity,
    const std::string &entityType,
    int entityId,
    const std::string &correlationId) {
    auditEvents.push_back({nextAuditEventId(), role, message, nowStamp(), action, severity, entityType, entityId, correlationId});
}

void FileDatabase::addNotification(
    const std::string &channel,
    const std::string &type,
    const std::string &message,
    const std::string &entityType,
    int entityId) {
    notifications.push_back({nextNotificationId(), channel, type, message, entityType, entityId, nowStamp(), "", false});
}

std::vector<NotificationRecord> FileDatabase::notificationsAfter(const std::string &channel, int afterId, int limit) const {
    std::vector<NotificationRecord> result;
    for (const NotificationRecord &notification : notifications) {
        if (notification.id <= afterId) {
            continue;
        }
        if (notification.channel == channel) {
            result.push_back(notification);
        }
        if (static_cast<int>(result.size()) >= limit) {
            break;
        }
    }
    return result;
}

}
