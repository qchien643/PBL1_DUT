#include "file_database.hpp"

#include "../shared/utils.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace app {

FileDatabase::FileDatabase(std::string filePath) : path(std::move(filePath)) {}

void FileDatabase::loadOrSeed() {
    if (!load()) {
        seed();
        save();
    }
}

bool FileDatabase::load() {
    std::ifstream input(path);
    if (!input) {
        return false;
    }

    tables.clear();
    sessions.clear();
    menuItems.clear();
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

    std::string section;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.front() == '[' && line.back() == ']') {
            section = line;
            continue;
        }

        const std::vector<std::string> fields = split(line, '|');
        if (section == "[TABLES]" && fields.size() >= 4) {
            tables.push_back({std::stoi(fields[0]), fields[1], fields[2], std::stoi(fields[3])});
        } else if (section == "[SESSIONS]" && fields.size() >= 5) {
            sessions.push_back({std::stoi(fields[0]), fields[1], parseIds(fields[2]), restore(fields[3]), restore(fields[4]), fields.size() >= 6 ? std::stoi(fields[5]) : 0});
        } else if (section == "[MENU_ITEMS]" && fields.size() >= 8) {
            menuItems.push_back({std::stoi(fields[0]), fields[1], restore(fields[2]), std::stoi(fields[3]), fields[4], fields[5], fields[6], std::stoi(fields[7])});
        } else if (section == "[ORDERS]" && fields.size() >= 5) {
            orders.push_back({std::stoi(fields[0]), std::stoi(fields[1]), fields[2], restore(fields[3]), restore(fields[4]), fields.size() >= 6 ? restore(fields[5]) : ""});
        } else if (section == "[ORDER_ITEMS]" && fields.size() >= 7) {
            orderItems.push_back({std::stoi(fields[0]), std::stoi(fields[1]), std::stoi(fields[2]), std::stoi(fields[3]), std::stoi(fields[4]), fields[5], restore(fields[6])});
        } else if (section == "[KITCHEN_TASKS]" && fields.size() >= 5) {
            kitchenTasks.push_back({std::stoi(fields[0]), std::stoi(fields[1]), fields[2], fields[3], restore(fields[4]), fields.size() >= 6 ? std::stoi(fields[5]) : 0});
        } else if (section == "[BILLS]" && fields.size() >= 5) {
            bills.push_back({
                std::stoi(fields[0]),
                std::stoi(fields[1]),
                std::stoi(fields[2]),
                fields[3],
                restore(fields[4]),
                fields.size() >= 6 ? std::stoi(fields[5]) : 1,
                fields.size() >= 7 ? std::stoi(fields[6]) : 0,
                fields.size() >= 8 ? std::stoi(fields[7]) : 0,
                fields.size() >= 9 ? restore(fields[8]) : "",
                fields.size() >= 10 ? restore(fields[9]) : "",
            });
        } else if (section == "[BILL_LINES]" && fields.size() >= 9) {
            billLines.push_back({std::stoi(fields[0]), std::stoi(fields[1]), std::stoi(fields[2]), std::stoi(fields[3]), restore(fields[4]), std::stoi(fields[5]), std::stoi(fields[6]), std::stoi(fields[7]), fields[8]});
        } else if (section == "[PAYMENTS]" && fields.size() >= 7) {
            payments.push_back({std::stoi(fields[0]), std::stoi(fields[1]), std::stoi(fields[2]), fields[3], fields[4], restore(fields[5]), restore(fields[6])});
        } else if (section == "[IDEMPOTENCY_KEYS]" && fields.size() >= 6) {
            idempotencyKeys.push_back({std::stoi(fields[0]), fields[1], restore(fields[2]), fields[3], std::stoi(fields[4]), restore(fields[5])});
        } else if (section == "[KITCHEN_ISSUES]" && fields.size() >= 8) {
            kitchenIssues.push_back({std::stoi(fields[0]), std::stoi(fields[1]), std::stoi(fields[2]), restore(fields[3]), restore(fields[4]), fields[5], restore(fields[6]), restore(fields[7])});
        } else if (section == "[STAFF_USERS]" && fields.size() >= 4) {
            staffUsers.push_back({std::stoi(fields[0]), fields[1], fields[2], fields[3] == "1"});
        } else if (section == "[ROLE_PERMISSIONS]" && fields.size() >= 3) {
            rolePermissions.push_back({std::stoi(fields[0]), fields[1], fields[2]});
        } else if (section == "[AUDIT_EVENTS]" && fields.size() >= 4) {
            auditEvents.push_back({
                std::stoi(fields[0]),
                fields[1],
                restore(fields[2]),
                restore(fields[3]),
                fields.size() >= 5 ? fields[4] : "",
                fields.size() >= 6 ? fields[5] : "LOW",
                fields.size() >= 7 ? restore(fields[6]) : "",
                fields.size() >= 8 ? std::stoi(fields[7]) : 0,
                fields.size() >= 9 ? restore(fields[8]) : "",
            });
        } else if (section == "[NOTIFICATIONS]" && fields.size() >= 7) {
            notifications.push_back({
                std::stoi(fields[0]),
                fields[1],
                fields[2],
                restore(fields[3]),
                restore(fields[4]),
                std::stoi(fields[5]),
                restore(fields[6]),
                fields.size() >= 8 ? restore(fields[7]) : "",
                fields.size() >= 9 && fields[8] == "1",
            });
        }
    }
    if (staffUsers.empty()) {
        staffUsers = {
            {1, "manager", "manager", true},
            {2, "cashier", "cashier", true},
            {3, "waiter", "waiter", true},
            {4, "kitchen", "kitchen", true},
            {5, "bar", "kitchen", true},
            {6, "customer", "customer", true},
        };
    }
    if (rolePermissions.empty()) {
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
        };
    }
    return true;
}

bool FileDatabase::save() const {
    const std::filesystem::path databasePath(path);
    if (databasePath.has_parent_path()) {
        std::filesystem::create_directories(databasePath.parent_path());
    }

    const std::string temporaryPath = path + ".tmp";
    std::ofstream output(temporaryPath);
    if (!output) {
        return false;
    }

    output << "[TABLES]\n";
    for (const TableRecord &table : tables) {
        output << table.id << '|' << table.code << '|' << table.state << '|' << table.activeSessionId << '\n';
    }

    output << "[SESSIONS]\n";
    for (const SessionRecord &session : sessions) {
        output << session.id << '|' << session.status << '|' << joinIds(session.tableIds) << '|'
               << sanitize(session.openedAt) << '|' << sanitize(session.closedAt) << '|'
               << session.version << '\n';
    }

    output << "[MENU_ITEMS]\n";
    for (const MenuItemRecord &item : menuItems) {
        output << item.id << '|' << item.category << '|' << sanitize(item.name) << '|' << item.price << '|'
               << item.catalogStatus << '|' << item.availabilityStatus << '|' << item.station << '|'
               << item.prepMinutes << '\n';
    }

    output << "[ORDERS]\n";
    for (const OrderRecord &order : orders) {
        output << order.id << '|' << order.sessionId << '|' << order.status << '|'
               << sanitize(order.note) << '|' << sanitize(order.createdAt) << '|'
               << sanitize(order.clientRequestId) << '\n';
    }

    output << "[ORDER_ITEMS]\n";
    for (const OrderItemRecord &item : orderItems) {
        output << item.id << '|' << item.orderId << '|' << item.menuItemId << '|' << item.quantity << '|'
               << item.unitPrice << '|' << item.status << '|' << sanitize(item.note) << '\n';
    }

    output << "[KITCHEN_TASKS]\n";
    for (const KitchenTaskRecord &task : kitchenTasks) {
        output << task.id << '|' << task.orderItemId << '|' << task.station << '|'
               << task.status << '|' << sanitize(task.issue) << '|' << task.issueId << '\n';
    }

    output << "[BILLS]\n";
    for (const BillRecord &bill : bills) {
        output << bill.id << '|' << bill.sessionId << '|' << bill.total << '|'
               << bill.status << '|' << sanitize(bill.paymentMethod) << '|'
               << bill.version << '|' << bill.sessionVersion << '|' << bill.paidAmount << '|'
               << sanitize(bill.paidAt) << '|' << sanitize(bill.idempotencyKey) << '\n';
    }

    output << "[BILL_LINES]\n";
    for (const BillLineRecord &line : billLines) {
        output << line.id << '|' << line.billId << '|' << line.orderItemId << '|'
               << line.menuItemId << '|' << sanitize(line.itemName) << '|'
               << line.quantity << '|' << line.unitPrice << '|' << line.lineTotal << '|'
               << line.sourceStatus << '\n';
    }

    output << "[PAYMENTS]\n";
    for (const PaymentRecord &payment : payments) {
        output << payment.id << '|' << payment.billId << '|' << payment.paidAmount << '|'
               << payment.method << '|' << payment.status << '|' << sanitize(payment.idempotencyKey)
               << '|' << sanitize(payment.paidAt) << '\n';
    }

    output << "[IDEMPOTENCY_KEYS]\n";
    for (const IdempotencyRecord &record : idempotencyKeys) {
        output << record.id << '|' << record.scope << '|' << sanitize(record.key) << '|'
               << record.operation << '|' << record.entityId << '|' << sanitize(record.createdAt) << '\n';
    }

    output << "[KITCHEN_ISSUES]\n";
    for (const KitchenIssueRecord &issue : kitchenIssues) {
        output << issue.id << '|' << issue.taskId << '|' << issue.orderItemId << '|'
               << sanitize(issue.reason) << '|' << sanitize(issue.resolution) << '|'
               << issue.status << '|' << sanitize(issue.createdAt) << '|'
               << sanitize(issue.resolvedAt) << '\n';
    }

    output << "[STAFF_USERS]\n";
    for (const StaffUserRecord &staff : staffUsers) {
        output << staff.id << '|' << staff.actor << '|' << staff.role << '|'
               << (staff.active ? "1" : "0") << '\n';
    }

    output << "[ROLE_PERMISSIONS]\n";
    for (const RolePermissionRecord &permission : rolePermissions) {
        output << permission.id << '|' << permission.role << '|' << permission.permissionKey << '\n';
    }

    output << "[AUDIT_EVENTS]\n";
    for (const AuditEventRecord &event : auditEvents) {
        output << event.id << '|' << event.role << '|' << sanitize(event.message) << '|'
               << sanitize(event.createdAt) << '|' << event.action << '|'
               << event.severity << '|' << sanitize(event.entityType) << '|'
               << event.entityId << '|' << sanitize(event.correlationId) << '\n';
    }

    output << "[NOTIFICATIONS]\n";
    for (const NotificationRecord &notification : notifications) {
        output << notification.id << '|' << notification.channel << '|' << notification.type << '|'
               << sanitize(notification.message) << '|' << sanitize(notification.entityType) << '|'
               << notification.entityId << '|' << sanitize(notification.createdAt) << '|'
               << sanitize(notification.correlationId) << '|'
               << (notification.read ? "1" : "0") << '\n';
    }

    output.close();
    if (!output) {
        return false;
    }

    std::error_code ignoredError;
    std::filesystem::remove(databasePath, ignoredError);
    std::filesystem::rename(temporaryPath, databasePath, ignoredError);
    return !ignoredError;
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
