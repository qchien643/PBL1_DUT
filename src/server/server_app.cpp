#include "server_app.hpp"

#include "http_types.hpp"
#include "json_helpers.hpp"
#include "simple_http_server.hpp"
#include "../infrastructure/file_database.hpp"
#include "../modules/kitchen_fulfillment/kitchen_fulfillment_service.hpp"
#include "../modules/menu_inventory/menu_inventory_service.hpp"
#include "../modules/order_management/order_management_service.hpp"
#include "../modules/payment_billing/payment_billing_service.hpp"
#include "../modules/recommendation_ai_ml/recommendation_service.hpp"
#include "../modules/reporting_audit/reporting_audit_service.hpp"
#include "../modules/table_session/table_session_service.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <sstream>
#include <utility>

namespace app::server {

namespace {

class ServerApplication {
public:
    explicit ServerApplication(std::string databasePath) : database(std::move(databasePath)) {
        database.loadOrSeed();
    }

    HttpResponse handle(const HttpRequest &request) {
        if (request.method == "OPTIONS") {
            return textResponse(200, "");
        }
        if (startsWith(request.path, "/api/")) {
            return handleApi(request);
        }
        return serveStatic(request.path);
    }

private:
    FileDatabase database;
    std::mutex databaseMutex;

    static bool startsWith(const std::string &value, const std::string &prefix) {
        return value.rfind(prefix, 0) == 0;
    }

    static int toInt(const std::string &value, int fallback = 0) {
        try {
            return std::stoi(value);
        } catch (...) {
            return fallback;
        }
    }

    static std::vector<std::string> splitPath(const std::string &path) {
        std::vector<std::string> parts;
        std::stringstream stream(path);
        std::string part;
        while (std::getline(stream, part, '/')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        return parts;
    }

    static std::string queryValue(const HttpRequest &request, const std::string &key, const std::string &fallback = "") {
        const auto iterator = request.query.find(key);
        return iterator == request.query.end() ? fallback : iterator->second;
    }

    static HttpResponse jsonResponse(int status, const std::string &body) {
        HttpResponse response;
        response.status = status;
        response.contentType = "application/json; charset=utf-8";
        response.body = body;
        return response;
    }

    static HttpResponse textResponse(int status, const std::string &body) {
        HttpResponse response;
        response.status = status;
        response.contentType = "text/plain; charset=utf-8";
        response.body = body;
        return response;
    }

    static HttpResponse success(const std::string &dataJson = "{}") {
        return jsonResponse(200, jsonSuccess(dataJson));
    }

    static HttpResponse failure(const std::string &code, const std::string &message, int status = 400) {
        return jsonResponse(status, jsonError(code, message));
    }

    static HttpResponse failure(const OperationResult &result, int status = 400) {
        return jsonResponse(status, jsonError(result.code.empty() ? "BUSINESS_RULE_DENIED" : result.code, result.message, result.requiredAction, result.contextJson, result.correlationId));
    }

    static std::string mimeType(const std::filesystem::path &path) {
        const std::string extension = path.extension().string();
        if (extension == ".html") {
            return "text/html; charset=utf-8";
        }
        if (extension == ".css") {
            return "text/css; charset=utf-8";
        }
        if (extension == ".js") {
            return "application/javascript; charset=utf-8";
        }
        if (extension == ".json") {
            return "application/json; charset=utf-8";
        }
        return "text/plain; charset=utf-8";
    }

    HttpResponse serveStatic(std::string path) {
        if (path == "/") {
            path = "/index.html";
        }
        if (path.find("..") != std::string::npos) {
            return failure("BAD_PATH", "Invalid static file path.", 400);
        }

        const std::filesystem::path filePath = std::filesystem::path("web") / path.substr(1);
        std::ifstream input(filePath, std::ios::binary);
        if (!input) {
            return failure("NOT_FOUND", "Static file not found.", 404);
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();

        HttpResponse response;
        response.status = 200;
        response.contentType = mimeType(filePath);
        response.body = buffer.str();
        return response;
    }

    HttpResponse handleApi(const HttpRequest &request) {
        std::lock_guard<std::mutex> lock(databaseMutex);
        database.loadOrSeed();

        const std::vector<std::string> parts = splitPath(request.path);
        if (parts.size() < 2 || parts[0] != "api") {
            return failure("NOT_FOUND", "API endpoint not found.", 404);
        }

        if (request.method == "GET" && parts.size() == 2 && parts[1] == "health") {
            return success("{\"status\":\"UP\",\"service\":\"casual-dining-server\"}");
        }
        if (request.method == "GET" && parts.size() == 2 && parts[1] == "tables") {
            return success("{\"tables\":" + tablesJson() + "}");
        }
        if (request.method == "GET" && parts.size() == 2 && parts[1] == "menu") {
            const bool includeHidden = queryValue(request, "includeHidden") == "true";
            return success("{\"items\":" + menuItemsJson(includeHidden) + "}");
        }
        if (request.method == "GET" && parts.size() == 4 && parts[1] == "tables" && parts[3] == "session") {
            return tableSessionResponse(parts[2]);
        }
        if (request.method == "GET" && parts.size() == 3 && parts[1] == "orders" && parts[2] == "pending") {
            return success("{\"orders\":" + pendingOrdersJson() + "}");
        }
        if (request.method == "GET" && parts.size() == 2 && parts[1] == "cancel-requests") {
            return success("{\"items\":" + cancelRequestsJson() + "}");
        }
        if (request.method == "GET" && parts.size() == 4 && parts[1] == "sessions" && parts[3] == "orders") {
            return success("{\"orders\":" + sessionOrdersJson(toInt(parts[2])) + "}");
        }
        if (request.method == "GET" && parts.size() == 4 && parts[1] == "sessions" && parts[3] == "recommendations") {
            return recommendationsResponse(toInt(parts[2]));
        }
        if (request.method == "GET" && parts.size() == 3 && parts[1] == "kitchen" && parts[2] == "tasks") {
            const std::string station = queryValue(request, "station", "kitchen");
            return success("{\"tasks\":" + kitchenTasksJson(station) + "}");
        }
        if (request.method == "GET" && parts.size() == 3 && parts[1] == "bills" && parts[2] == "open") {
            return success("{\"bills\":" + openBillsJson() + "}");
        }
        if (request.method == "GET" && parts.size() == 2 && parts[1] == "notifications") {
            return notificationsResponse(queryValue(request, "channel"), toInt(queryValue(request, "after", "0")), toInt(queryValue(request, "limit", "50"), 50));
        }
        if (request.method == "GET" && parts.size() == 3 && parts[1] == "reports" && parts[2] == "summary") {
            return reportsSummaryResponse();
        }
        if (request.method == "GET" && parts.size() == 2 && parts[1] == "audit-events") {
            return success("{\"events\":" + auditEventsJson(toInt(queryValue(request, "limit", "50"), 50)) + "}");
        }
        if (request.method == "GET" && parts.size() == 3 && parts[1] == "staff" && parts[2] == "permissions") {
            return permissionsResponse(queryValue(request, "actor", "customer"));
        }

        if (request.method == "POST" && parts.size() == 4 && parts[1] == "tables" && parts[3] == "open") {
            return openTableResponse(parts[2]);
        }
        if (request.method == "POST" && parts.size() == 3 && parts[1] == "tables" && parts[2] == "merge") {
            return mergeTablesResponse(request.body);
        }
        if (request.method == "POST" && parts.size() == 3 && parts[1] == "tables" && parts[2] == "transfer") {
            return transferTableResponse(request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "tables" && parts[3] == "cleaned") {
            return operationResponse(table_session::markTableCleaned(database, parts[2], "cashier"), "TABLE_CLEANED");
        }
        if (request.method == "PATCH" && parts.size() == 4 && parts[1] == "menu" && parts[3] == "availability") {
            return setMenuAvailabilityResponse(toInt(parts[2]), request.body);
        }
        if (request.method == "POST" && parts.size() == 2 && parts[1] == "orders") {
            return submitOrderResponse(request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "orders" && parts[3] == "accept") {
            return acceptOrderResponse(toInt(parts[2]));
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "orders" && parts[3] == "reject") {
            return rejectOrderResponse(toInt(parts[2]));
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "orders" && parts[3] == "customer-decision") {
            return customerDecisionResponse(toInt(parts[2]), request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "order-items" && parts[3] == "cancel-request") {
            return cancelRequestResponse(toInt(parts[2]), request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "order-items" && parts[3] == "cancel-approve") {
            return approveCancelResponse(toInt(parts[2]));
        }
        if (request.method == "POST" && parts.size() == 5 && parts[1] == "kitchen" && parts[2] == "tasks" && parts[4] == "start") {
            return startTaskResponse(toInt(parts[3]), request.body);
        }
        if (request.method == "POST" && parts.size() == 5 && parts[1] == "kitchen" && parts[2] == "tasks" && parts[4] == "ready") {
            return readyTaskResponse(toInt(parts[3]), request.body);
        }
        if (request.method == "POST" && parts.size() == 5 && parts[1] == "kitchen" && parts[2] == "tasks" && parts[4] == "served") {
            return servedTaskResponse(toInt(parts[3]), request.body);
        }
        if (request.method == "POST" && parts.size() == 5 && parts[1] == "kitchen" && parts[2] == "tasks" && parts[4] == "issue") {
            return reportIssueResponse(toInt(parts[3]), request.body);
        }
        if (request.method == "POST" && parts.size() == 5 && parts[1] == "kitchen" && parts[2] == "issues" && parts[4] == "resolve") {
            return resolveIssueResponse(toInt(parts[3]), request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "sessions" && parts[3] == "bill") {
            return createBillResponse(toInt(parts[2]), request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "bills" && parts[3] == "pay") {
            return payBillResponse(toInt(parts[2]), request.body);
        }
        if (request.method == "POST" && parts.size() == 4 && parts[1] == "bills" && parts[3] == "reopen") {
            return reopenBillResponse(toInt(parts[2]), request.body);
        }

        return failure("NOT_FOUND", "API endpoint not found.", 404);
    }

    HttpResponse operationResponse(const OperationResult &result, const std::string &code) {
        if (!result.ok) {
            OperationResult enriched = result;
            if (enriched.code.empty()) {
                enriched.code = code;
            }
            return failure(enriched);
        }
        return success("{\"id\":" + std::to_string(result.id) +
                       ",\"message\":" + jsonString(result.message) +
                       ",\"code\":" + jsonString(result.code) +
                       ",\"requiredAction\":" + jsonString(result.requiredAction) +
                       ",\"context\":" + (result.contextJson.empty() ? "{}" : result.contextJson) + "}");
    }

    std::string tableCodesJson(const SessionRecord &session) {
        std::string json = "[";
        for (size_t index = 0; index < session.tableIds.size(); ++index) {
            TableRecord *table = database.findTableById(session.tableIds[index]);
            if (table == nullptr) {
                continue;
            }
            if (json.size() > 1) {
                json += ",";
            }
            json += jsonString(table->code);
        }
        json += "]";
        return json;
    }

    std::string tableCodesText(const SessionRecord &session) {
        return table_session::tableCodesForSession(database, session);
    }

    std::string tablesJson() {
        std::string json = "[";
        for (size_t index = 0; index < database.tables.size(); ++index) {
            TableRecord &table = database.tables[index];
            if (index > 0) {
                json += ",";
            }
            std::string joinedTables = "[]";
            if (table.activeSessionId != 0) {
                SessionRecord *session = database.findSessionById(table.activeSessionId);
                if (session != nullptr) {
                    joinedTables = tableCodesJson(*session);
                }
            }
            json += "{\"id\":" + std::to_string(table.id) +
                    ",\"code\":" + jsonString(table.code) +
                    ",\"state\":" + jsonString(table.state) +
                    ",\"activeSessionId\":" + std::to_string(table.activeSessionId) +
                    ",\"joinedTables\":" + joinedTables + "}";
        }
        json += "]";
        return json;
    }

    std::string menuItemsJson(bool includeHidden) {
        std::string json = "[";
        bool first = true;
        for (const MenuItemRecord &item : menu_inventory::visibleMenuItems(database, includeHidden)) {
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"id\":" + std::to_string(item.id) +
                    ",\"category\":" + jsonString(item.category) +
                    ",\"name\":" + jsonString(item.name) +
                    ",\"price\":" + std::to_string(item.price) +
                    ",\"catalogStatus\":" + jsonString(item.catalogStatus) +
                    ",\"availabilityStatus\":" + jsonString(item.availabilityStatus) +
                    ",\"station\":" + jsonString(item.station) +
                    ",\"prepMinutes\":" + std::to_string(item.prepMinutes) + "}";
        }
        json += "]";
        return json;
    }

    std::string orderItemsJson(int orderId) {
        std::string json = "[";
        bool first = true;
        for (const OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.orderId != orderId) {
                continue;
            }
            MenuItemRecord *item = database.findMenuItemById(orderItem.menuItemId);
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"id\":" + std::to_string(orderItem.id) +
                    ",\"menuItemId\":" + std::to_string(orderItem.menuItemId) +
                    ",\"name\":" + jsonString(item == nullptr ? "Unknown item" : item->name) +
                    ",\"quantity\":" + std::to_string(orderItem.quantity) +
                    ",\"unitPrice\":" + std::to_string(orderItem.unitPrice) +
                    ",\"status\":" + jsonString(orderItem.status) +
                    ",\"note\":" + jsonString(orderItem.note) +
                    ",\"availabilityStatus\":" + jsonString(item == nullptr ? "UNKNOWN" : item->availabilityStatus) +
                    ",\"lineTotal\":" + std::to_string(orderItem.quantity * orderItem.unitPrice) + "}";
        }
        json += "]";
        return json;
    }

    std::string orderJson(const OrderRecord &order) {
        SessionRecord *session = database.findSessionById(order.sessionId);
        return "{\"id\":" + std::to_string(order.id) +
               ",\"sessionId\":" + std::to_string(order.sessionId) +
               ",\"status\":" + jsonString(order.status) +
               ",\"createdAt\":" + jsonString(order.createdAt) +
               ",\"tableCodes\":" + (session == nullptr ? "[]" : tableCodesJson(*session)) +
               ",\"items\":" + orderItemsJson(order.id) + "}";
    }

    std::string pendingOrdersJson() {
        std::string json = "[";
        bool first = true;
        for (const OrderRecord &order : database.orders) {
            if (order.status != "SUBMITTED") {
                continue;
            }
            if (!first) {
                json += ",";
            }
            first = false;
            json += orderJson(order);
        }
        json += "]";
        return json;
    }

    std::string sessionOrdersJson(int sessionId) {
        std::string json = "[";
        bool first = true;
        for (const OrderRecord &order : database.orders) {
            if (order.sessionId != sessionId) {
                continue;
            }
            if (!first) {
                json += ",";
            }
            first = false;
            json += orderJson(order);
        }
        json += "]";
        return json;
    }

    std::string cancelRequestsJson() {
        std::string json = "[";
        bool first = true;
        for (const OrderItemRecord &orderItem : database.orderItems) {
            if (orderItem.status != "CANCEL_REQUESTED") {
                continue;
            }
            OrderRecord *order = database.findOrderById(orderItem.orderId);
            MenuItemRecord *item = database.findMenuItemById(orderItem.menuItemId);
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"orderItemId\":" + std::to_string(orderItem.id) +
                    ",\"orderId\":" + std::to_string(orderItem.orderId) +
                    ",\"sessionId\":" + std::to_string(order == nullptr ? 0 : order->sessionId) +
                    ",\"name\":" + jsonString(item == nullptr ? "Unknown item" : item->name) +
                    ",\"quantity\":" + std::to_string(orderItem.quantity) + "}";
        }
        json += "]";
        return json;
    }

    std::string kitchenTasksJson(const std::string &station) {
        std::string json = "[";
        bool first = true;
        for (const KitchenTaskRecord &task : database.kitchenTasks) {
            if (task.station != station || task.status == "CANCELLED") {
                continue;
            }
            OrderItemRecord *orderItem = database.findOrderItemById(task.orderItemId);
            if (orderItem == nullptr) {
                continue;
            }
            MenuItemRecord *menuItem = database.findMenuItemById(orderItem->menuItemId);
            OrderRecord *order = database.findOrderById(orderItem->orderId);
            SessionRecord *session = order == nullptr ? nullptr : database.findSessionById(order->sessionId);
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"id\":" + std::to_string(task.id) +
                    ",\"orderItemId\":" + std::to_string(task.orderItemId) +
                    ",\"station\":" + jsonString(task.station) +
                    ",\"status\":" + jsonString(task.status) +
                    ",\"issueId\":" + std::to_string(task.issueId) +
                    ",\"issue\":" + jsonString(task.issue) +
                    ",\"itemName\":" + jsonString(menuItem == nullptr ? "Unknown item" : menuItem->name) +
                    ",\"quantity\":" + std::to_string(orderItem->quantity) +
                    ",\"tableCodes\":" + (session == nullptr ? "[]" : tableCodesJson(*session)) + "}";
        }
        json += "]";
        return json;
    }

    std::string openBillsJson() {
        std::string json = "[";
        bool first = true;
        for (const BillRecord &bill : database.bills) {
            if (bill.status != "OPEN") {
                continue;
            }
            SessionRecord *session = database.findSessionById(bill.sessionId);
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"id\":" + std::to_string(bill.id) +
                    ",\"sessionId\":" + std::to_string(bill.sessionId) +
                    ",\"total\":" + std::to_string(bill.total) +
                    ",\"status\":" + jsonString(bill.status) +
                    ",\"version\":" + std::to_string(bill.version) +
                    ",\"sessionVersion\":" + std::to_string(bill.sessionVersion) +
                    ",\"tableCodes\":" + (session == nullptr ? "[]" : tableCodesJson(*session)) + "}";
        }
        json += "]";
        return json;
    }

    std::string auditEventsJson(int limit) {
        std::string json = "[";
        bool first = true;
        for (const AuditEventRecord &event : reporting_audit::recentAuditEvents(database, limit)) {
            if (!first) {
                json += ",";
            }
            first = false;
            json += "{\"id\":" + std::to_string(event.id) +
                    ",\"role\":" + jsonString(event.role) +
                    ",\"message\":" + jsonString(event.message) +
                    ",\"action\":" + jsonString(event.action) +
                    ",\"severity\":" + jsonString(event.severity) +
                    ",\"entityType\":" + jsonString(event.entityType) +
                    ",\"entityId\":" + std::to_string(event.entityId) +
                    ",\"createdAt\":" + jsonString(event.createdAt) + "}";
        }
        json += "]";
        return json;
    }

    std::string notificationsJson(const std::vector<NotificationRecord> &events) {
        std::string json = "[";
        for (size_t index = 0; index < events.size(); ++index) {
            const NotificationRecord &event = events[index];
            if (index > 0) {
                json += ",";
            }
            json += "{\"id\":" + std::to_string(event.id) +
                    ",\"channel\":" + jsonString(event.channel) +
                    ",\"type\":" + jsonString(event.type) +
                    ",\"message\":" + jsonString(event.message) +
                    ",\"entityType\":" + jsonString(event.entityType) +
                    ",\"entityId\":" + std::to_string(event.entityId) +
                    ",\"correlationId\":" + jsonString(event.correlationId) +
                    ",\"createdAt\":" + jsonString(event.createdAt) + "}";
        }
        json += "]";
        return json;
    }

    HttpResponse tableSessionResponse(const std::string &tableCode) {
        SessionRecord *session = database.findActiveSessionByTableCode(tableCode);
        if (session == nullptr) {
            return success("{\"hasActiveSession\":false,\"session\":null}");
        }
        return success("{\"hasActiveSession\":true,\"session\":{\"id\":" + std::to_string(session->id) +
                       ",\"status\":" + jsonString(session->status) +
                       ",\"tableCodes\":" + tableCodesJson(*session) + "}}");
    }

    HttpResponse recommendationsResponse(int sessionId) {
        SessionRecord *session = database.findSessionById(sessionId);
        if (session == nullptr) {
            return failure("SESSION_NOT_FOUND", "Dining session not found.", 404);
        }
        std::string json = "[";
        const std::vector<recommendation_ai_ml::Recommendation> recommendations = recommendation_ai_ml::recommendForSession(database, *session, 5);
        for (size_t index = 0; index < recommendations.size(); ++index) {
            MenuItemRecord *item = database.findMenuItemById(recommendations[index].itemId);
            if (item == nullptr) {
                continue;
            }
            if (json.size() > 1) {
                json += ",";
            }
            const int matchPercent = std::min(99, 60 + static_cast<int>(recommendations[index].score * 25));
            json += "{\"menuItemId\":" + std::to_string(item->id) +
                    ",\"name\":" + jsonString(item->name) +
                    ",\"price\":" + std::to_string(item->price) +
                    ",\"matchPercent\":" + std::to_string(matchPercent) +
                    ",\"reason\":\"Pairs well with current table order\"}";
        }
        json += "]";
        return success("{\"items\":" + json + "}");
    }

    HttpResponse notificationsResponse(const std::string &channel, int afterId, int limit) {
        if (channel.empty()) {
            return failure("INVALID_CHANNEL", "Notification channel is required.");
        }
        return success("{\"events\":" + notificationsJson(database.notificationsAfter(channel, afterId, limit <= 0 ? 50 : limit)) + "}");
    }

    HttpResponse reportsSummaryResponse() {
        int orderCount = 0;
        for (const OrderRecord &order : database.orders) {
            if (order.status != "REJECTED") {
                ++orderCount;
            }
        }
        return success("{\"paidRevenue\":" + std::to_string(reporting_audit::paidRevenue(database)) +
                       ",\"orderCount\":" + std::to_string(orderCount) +
                       ",\"tableCount\":" + std::to_string(database.tables.size()) + "}");
    }

    HttpResponse permissionsResponse(const std::string &actor) {
        const std::string role = database.roleForActor(actor);
        std::string permissions = "[";
        bool first = true;
        for (const RolePermissionRecord &permission : database.rolePermissions) {
            if (permission.role != role && permission.role != "manager") {
                continue;
            }
            if (role != "manager" && permission.role == "manager") {
                continue;
            }
            if (!first) {
                permissions += ",";
            }
            first = false;
            permissions += jsonString(permission.permissionKey);
        }
        permissions += "]";
        return success("{\"actor\":" + jsonString(actor) + ",\"role\":" + jsonString(role) + ",\"permissions\":" + permissions + "}");
    }

    void notifySessionCustomers(int sessionId, const std::string &type, const std::string &message, const std::string &entityType, int entityId) {
        SessionRecord *session = database.findSessionById(sessionId);
        if (session == nullptr) {
            return;
        }
        for (int tableId : session->tableIds) {
            TableRecord *table = database.findTableById(tableId);
            if (table != nullptr) {
                database.addNotification("customer:" + table->code, type, message, entityType, entityId);
            }
        }
    }

    HttpResponse openTableResponse(const std::string &tableCode) {
        const OperationResult result = table_session::openTable(database, tableCode, "cashier");
        if (!result.ok) {
            return failure("TABLE_OPEN_FAILED", result.message);
        }
        database.addNotification("customer:" + tableCode, "TABLE_OPENED", "Table " + tableCode + " is ready to order.", "session", result.id);
        database.save();
        return operationResponse(result, "TABLE_OPEN_FAILED");
    }

    HttpResponse mergeTablesResponse(const std::string &body) {
        const std::string mainTableCode = extractJsonString(body, "mainTableCode");
        const std::string joinedTableCode = extractJsonString(body, "joinedTableCode");
        return operationResponse(table_session::mergeTables(database, mainTableCode, joinedTableCode, "cashier"), "TABLE_MERGE_FAILED");
    }

    HttpResponse transferTableResponse(const std::string &body) {
        const std::string sourceTableCode = extractJsonString(body, "sourceTableCode");
        const std::string targetTableCode = extractJsonString(body, "targetTableCode");
        return operationResponse(table_session::transferTable(database, sourceTableCode, targetTableCode, "cashier"), "TABLE_TRANSFER_FAILED");
    }

    HttpResponse setMenuAvailabilityResponse(int menuItemId, const std::string &body) {
        const std::string availabilityStatus = extractJsonString(body, "availabilityStatus");
        const std::string actor = extractJsonString(body, "actor", "manager");
        const OperationResult result = menu_inventory::setAvailability(database, menuItemId, availabilityStatus, actor);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "MENU_CHANGED", "Menu availability changed.", "menu_item", menuItemId);
        database.save();
        return operationResponse(result, "MENU_UPDATE_FAILED");
    }

    HttpResponse submitOrderResponse(const std::string &body) {
        const int sessionId = extractJsonInt(body, "sessionId");
        const std::string tableCode = extractJsonString(body, "tableCode");
        const std::string idempotencyKey = extractJsonString(body, "idempotencyKey");
        const OperationResult result = order_management::submitOrder(database, sessionId, extractCartLines(body), "customer", idempotencyKey);
        if (!result.ok) {
            return failure(result);
        }
        if (result.code != "IDEMPOTENT_REPLAY") {
            database.addNotification("cashier", "NEW_ORDER", "Table " + tableCode + " submitted order #" + std::to_string(result.id) + ".", "order", result.id);
        }
        database.save();
        return success("{\"orderId\":" + std::to_string(result.id) + ",\"status\":\"SUBMITTED\",\"message\":" + jsonString(result.message) + "}");
    }

    HttpResponse acceptOrderResponse(int orderId) {
        OrderRecord *orderBefore = database.findOrderById(orderId);
        const int sessionId = orderBefore == nullptr ? 0 : orderBefore->sessionId;
        const OperationResult result = order_management::acceptOrder(database, orderId, "cashier");
        if (!result.ok) {
            return failure(result);
        }

        if (result.code == "UNAVAILABLE_ITEM_REQUIRES_CUSTOMER_CONFIRMATION") {
            notifySessionCustomers(sessionId, "ORDER_NEEDS_CUSTOMER_CONFIRMATION", "Order #" + std::to_string(orderId) + " has sold-out item(s). Please choose what to do.", "order", orderId);
            database.addNotification("cashier", "ORDER_NEEDS_CUSTOMER_CONFIRMATION", "Waiting for customer decision on order #" + std::to_string(orderId) + ".", "order", orderId);
            database.save();
            return operationResponse(result, "ORDER_ACCEPT_FAILED");
        }

        notifySessionCustomers(sessionId, "ORDER_ACCEPTED", "Order #" + std::to_string(orderId) + " was accepted.", "order", orderId);
        std::set<std::string> stations;
        for (const KitchenTaskRecord &task : database.kitchenTasks) {
            OrderItemRecord *item = database.findOrderItemById(task.orderItemId);
            if (item != nullptr && item->orderId == orderId && task.status == "PENDING") {
                stations.insert(task.station);
            }
        }
        SessionRecord *session = database.findSessionById(sessionId);
        const std::string tableText = session == nullptr ? "-" : tableCodesText(*session);
        for (const std::string &station : stations) {
            database.addNotification(station, "TASK_CREATED", "New task from table " + tableText + ".", "order", orderId);
        }
        database.save();
        return operationResponse(result, "ORDER_ACCEPT_FAILED");
    }

    HttpResponse rejectOrderResponse(int orderId) {
        OrderRecord *orderBefore = database.findOrderById(orderId);
        const int sessionId = orderBefore == nullptr ? 0 : orderBefore->sessionId;
        const OperationResult result = order_management::rejectOrder(database, orderId, "cashier");
        if (!result.ok) {
            return failure(result);
        }
        notifySessionCustomers(sessionId, "ORDER_REJECTED", "Order #" + std::to_string(orderId) + " was rejected.", "order", orderId);
        database.save();
        return operationResponse(result, "ORDER_REJECT_FAILED");
    }

    HttpResponse customerDecisionResponse(int orderId, const std::string &body) {
        const std::string decision = extractJsonString(body, "decision");
        const OperationResult result = order_management::resolveCustomerDecision(database, orderId, decision, extractCartLines(body), "customer");
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "CUSTOMER_DECISION_APPLIED", "Customer updated order #" + std::to_string(orderId) + ". Please review again.", "order", orderId);
        notifySessionCustomers(database.findOrderById(orderId) == nullptr ? 0 : database.findOrderById(orderId)->sessionId, "CUSTOMER_DECISION_APPLIED", "Your order decision was saved.", "order", orderId);
        database.save();
        return operationResponse(result, "CUSTOMER_DECISION_FAILED");
    }

    HttpResponse cancelRequestResponse(int orderItemId, const std::string &body) {
        const int sessionId = extractJsonInt(body, "sessionId");
        const OperationResult result = order_management::requestCancel(database, sessionId, orderItemId, "customer");
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "CANCEL_REQUESTED", "Customer requested cancel for item #" + std::to_string(orderItemId) + ".", "order_item", orderItemId);
        database.save();
        return operationResponse(result, "CANCEL_REQUEST_FAILED");
    }

    HttpResponse approveCancelResponse(int orderItemId) {
        OrderItemRecord *item = database.findOrderItemById(orderItemId);
        OrderRecord *order = item == nullptr ? nullptr : database.findOrderById(item->orderId);
        const int sessionId = order == nullptr ? 0 : order->sessionId;
        const OperationResult result = order_management::approveCancel(database, orderItemId, "cashier");
        if (!result.ok) {
            return failure(result);
        }
        notifySessionCustomers(sessionId, "CANCEL_APPROVED", "Cancel request for item #" + std::to_string(orderItemId) + " was approved.", "order_item", orderItemId);
        database.save();
        return operationResponse(result, "CANCEL_APPROVE_FAILED");
    }

    HttpResponse startTaskResponse(int taskId, const std::string &body) {
        const std::string station = extractJsonString(body, "station", "kitchen");
        return operationResponse(kitchen_fulfillment::startTask(database, taskId, station, station), "TASK_START_FAILED");
    }

    HttpResponse readyTaskResponse(int taskId, const std::string &body) {
        const std::string station = extractJsonString(body, "station", "kitchen");
        KitchenTaskRecord *taskBefore = nullptr;
        for (KitchenTaskRecord &task : database.kitchenTasks) {
            if (task.id == taskId) {
                taskBefore = &task;
                break;
            }
        }
        OrderItemRecord *item = taskBefore == nullptr ? nullptr : database.findOrderItemById(taskBefore->orderItemId);
        OrderRecord *order = item == nullptr ? nullptr : database.findOrderById(item->orderId);
        const int sessionId = order == nullptr ? 0 : order->sessionId;

        const OperationResult result = kitchen_fulfillment::completeTask(database, taskId, station, station);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "TASK_READY", station + " marked task #" + std::to_string(taskId) + " ready.", "kitchen_task", taskId);
        notifySessionCustomers(sessionId, "TASK_READY", "An item from your order is ready.", "kitchen_task", taskId);
        database.save();
        return operationResponse(result, "TASK_READY_FAILED");
    }

    HttpResponse servedTaskResponse(int taskId, const std::string &body) {
        const std::string actor = extractJsonString(body, "actor", "waiter");
        KitchenTaskRecord *taskBefore = database.findKitchenTaskById(taskId);
        OrderItemRecord *item = taskBefore == nullptr ? nullptr : database.findOrderItemById(taskBefore->orderItemId);
        OrderRecord *order = item == nullptr ? nullptr : database.findOrderById(item->orderId);
        const int sessionId = order == nullptr ? 0 : order->sessionId;

        const OperationResult result = kitchen_fulfillment::markServed(database, taskId, actor);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "TASK_SERVED", "Task #" + std::to_string(taskId) + " was served.", "kitchen_task", taskId);
        notifySessionCustomers(sessionId, "TASK_SERVED", "An item was served to your table.", "kitchen_task", taskId);
        database.save();
        return operationResponse(result, "TASK_SERVED_FAILED");
    }

    HttpResponse reportIssueResponse(int taskId, const std::string &body) {
        const std::string station = extractJsonString(body, "station", "kitchen");
        const std::string reason = extractJsonString(body, "reason", "Kitchen issue");
        KitchenTaskRecord *taskBefore = database.findKitchenTaskById(taskId);
        OrderItemRecord *item = taskBefore == nullptr ? nullptr : database.findOrderItemById(taskBefore->orderItemId);
        OrderRecord *order = item == nullptr ? nullptr : database.findOrderById(item->orderId);
        const int sessionId = order == nullptr ? 0 : order->sessionId;

        const OperationResult result = kitchen_fulfillment::reportIssue(database, taskId, station, reason, station);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "KITCHEN_ISSUE", "Kitchen issue on task #" + std::to_string(taskId) + ": " + reason, "kitchen_issue", result.id);
        database.addNotification("manager", "KITCHEN_ISSUE", "Kitchen issue requires resolution.", "kitchen_issue", result.id);
        notifySessionCustomers(sessionId, "KITCHEN_ISSUE", "A kitchen issue affected one item. Staff will resolve it.", "kitchen_issue", result.id);
        database.save();
        return operationResponse(result, "KITCHEN_ISSUE_FAILED");
    }

    HttpResponse resolveIssueResponse(int issueId, const std::string &body) {
        const std::string actor = extractJsonString(body, "actor", "cashier");
        const std::string resolution = extractJsonString(body, "resolution");
        KitchenIssueRecord *issueBefore = database.findKitchenIssueById(issueId);
        OrderRecord *order = nullptr;
        if (issueBefore != nullptr) {
            OrderItemRecord *item = database.findOrderItemById(issueBefore->orderItemId);
            order = item == nullptr ? nullptr : database.findOrderById(item->orderId);
        }
        const int sessionId = order == nullptr ? 0 : order->sessionId;

        const OperationResult result = kitchen_fulfillment::resolveIssue(database, issueId, resolution, actor);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "KITCHEN_ISSUE_RESOLVED", "Kitchen issue #" + std::to_string(issueId) + " was resolved.", "kitchen_issue", issueId);
        notifySessionCustomers(sessionId, "KITCHEN_ISSUE_RESOLVED", "A kitchen issue was resolved.", "kitchen_issue", issueId);
        database.save();
        return operationResponse(result, "KITCHEN_ISSUE_RESOLVE_FAILED");
    }

    HttpResponse createBillResponse(int sessionId, const std::string &body) {
        const std::string actor = extractJsonString(body, "actor", "customer");
        const OperationResult result = payment_billing::createBill(database, sessionId, actor);
        if (!result.ok) {
            return failure(result);
        }
        database.addNotification("cashier", "BILL_REQUESTED", "Bill requested for session #" + std::to_string(sessionId) + ".", "bill", result.id);
        database.save();
        BillRecord *bill = nullptr;
        for (BillRecord &candidate : database.bills) {
            if (candidate.id == result.id) {
                bill = &candidate;
                break;
            }
        }
        return success("{\"billId\":" + std::to_string(result.id) +
                       ",\"total\":" + std::to_string(bill == nullptr ? 0 : bill->total) +
                       ",\"version\":" + std::to_string(bill == nullptr ? 0 : bill->version) + "}");
    }

    HttpResponse payBillResponse(int billId, const std::string &body) {
        BillRecord *billBefore = nullptr;
        for (BillRecord &bill : database.bills) {
            if (bill.id == billId) {
                billBefore = &bill;
                break;
            }
        }
        const int sessionId = billBefore == nullptr ? 0 : billBefore->sessionId;
        const std::string actor = extractJsonString(body, "actor", "cashier");
        const OperationResult result = payment_billing::confirmPayment(
            database,
            billId,
            extractJsonString(body, "paymentMethod", "cash"),
            extractJsonInt(body, "paidAmount", billBefore == nullptr ? 0 : billBefore->total),
            extractJsonInt(body, "billVersion", billBefore == nullptr ? 0 : billBefore->version),
            extractJsonString(body, "idempotencyKey"),
            actor);
        if (!result.ok) {
            return failure(result);
        }
        notifySessionCustomers(sessionId, "BILL_PAID", "Bill #" + std::to_string(billId) + " has been paid.", "bill", billId);
        database.addNotification("manager", "BILL_PAID", "Bill #" + std::to_string(billId) + " was paid.", "bill", billId);
        database.save();
        return operationResponse(result, "BILL_PAY_FAILED");
    }

    HttpResponse reopenBillResponse(int billId, const std::string &body) {
        const std::string actor = extractJsonString(body, "actor", "cashier");
        BillRecord *billBefore = database.findBillById(billId);
        const int sessionId = billBefore == nullptr ? 0 : billBefore->sessionId;
        const OperationResult result = payment_billing::reopenBill(database, billId, actor);
        if (!result.ok) {
            return failure(result);
        }
        notifySessionCustomers(sessionId, "BILL_REOPENED", "Bill was reopened. You can order more items.", "bill", billId);
        database.addNotification("cashier", "BILL_REOPENED", "Bill #" + std::to_string(billId) + " was reopened.", "bill", billId);
        database.save();
        return operationResponse(result, "BILL_REOPEN_FAILED");
    }
};

}

int runWebServer(int port) {
    ServerApplication application("data/restaurant_db.txt");
    SimpleHttpServer server([&application](const HttpRequest &request) {
        return application.handle(request);
    });
    return server.listen(port) ? 0 : 1;
}

}
