#include "table_session_service.hpp"

#include "../../policies/business_policies.hpp"
#include "../../shared/utils.hpp"

#include <algorithm>

namespace app::table_session {

std::string tableCodesForSession(FileDatabase &database, const SessionRecord &session) {
    std::vector<std::string> codes;
    for (int tableId : session.tableIds) {
        TableRecord *table = database.findTableById(tableId);
        if (table != nullptr) {
            codes.push_back(table->code);
        }
    }

    std::string result;
    for (size_t index = 0; index < codes.size(); ++index) {
        if (index > 0) {
            result += "+";
        }
        result += codes[index];
    }
    return result.empty() ? "-" : result;
}

OperationResult openTable(FileDatabase &database, const std::string &tableCode, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "table.open");
    if (!permission.ok) {
        return permission;
    }

    TableRecord *table = database.findTableByCode(tableCode);
    if (table == nullptr) {
        return OperationResult::failure("Table not found.");
    }
    if (!policy::canOpenTable(*table)) {
        return OperationResult::failure("Table is not available.");
    }

    SessionRecord session;
    session.id = database.nextSessionId();
    session.status = "ACTIVE";
    session.tableIds = {table->id};
    session.openedAt = nowStamp();
    database.sessions.push_back(session);

    table->state = "OCCUPIED";
    table->activeSessionId = session.id;
    database.addAudit(actor, "Opened session #" + std::to_string(session.id) + " for table " + table->code);
    database.save();
    return OperationResult::success("Opened session #" + std::to_string(session.id) + ".", session.id);
}

OperationResult mergeTables(FileDatabase &database, const std::string &mainTableCode, const std::string &joinedTableCode, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "table.merge");
    if (!permission.ok) {
        return permission;
    }

    TableRecord *mainTable = database.findTableByCode(mainTableCode);
    TableRecord *joinedTable = database.findTableByCode(joinedTableCode);
    if (mainTable == nullptr || joinedTable == nullptr || mainTable->id == joinedTable->id) {
        return OperationResult::failure("Invalid tables.");
    }
    if (mainTable->activeSessionId == 0) {
        return OperationResult::failure("Main table must have an active session.");
    }

    SessionRecord *mainSession = database.findSessionById(mainTable->activeSessionId);
    if (mainSession == nullptr || mainSession->status == "CLOSED") {
        return OperationResult::failure("Main session is invalid.");
    }
    if (mainSession->status == "BILL_REQUESTED") {
        return OperationResult::failure("Cannot merge tables after bill has been requested.", "SESSION_LOCKED_FOR_BILLING", "REOPEN_BILL_FIRST");
    }

    if (joinedTable->activeSessionId == 0) {
        mainSession->tableIds.push_back(joinedTable->id);
        joinedTable->state = "OCCUPIED";
        joinedTable->activeSessionId = mainSession->id;
    } else {
        SessionRecord *joinedSession = database.findSessionById(joinedTable->activeSessionId);
        if (joinedSession == nullptr || joinedSession->status == "CLOSED") {
            return OperationResult::failure("Joined session is invalid.");
        }

        for (int tableId : joinedSession->tableIds) {
            if (std::find(mainSession->tableIds.begin(), mainSession->tableIds.end(), tableId) == mainSession->tableIds.end()) {
                mainSession->tableIds.push_back(tableId);
            }
            TableRecord *table = database.findTableById(tableId);
            if (table != nullptr) {
                table->activeSessionId = mainSession->id;
                table->state = "OCCUPIED";
            }
        }

        for (OrderRecord &order : database.orders) {
            if (order.sessionId == joinedSession->id) {
                order.sessionId = mainSession->id;
            }
        }
        joinedSession->status = "CLOSED";
        joinedSession->closedAt = nowStamp();
    }

    database.addAudit(actor, "Merged table " + joinedTable->code + " into session #" + std::to_string(mainSession->id));
    database.save();
    return OperationResult::success("Tables merged into session #" + std::to_string(mainSession->id) + ".", mainSession->id);
}

OperationResult transferTable(FileDatabase &database, const std::string &sourceCode, const std::string &targetCode, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "table.transfer");
    if (!permission.ok) {
        return permission;
    }

    TableRecord *sourceTable = database.findTableByCode(sourceCode);
    TableRecord *targetTable = database.findTableByCode(targetCode);
    if (sourceTable == nullptr || targetTable == nullptr || sourceTable->activeSessionId == 0) {
        return OperationResult::failure("Invalid source/target table.");
    }
    if (!policy::canOpenTable(*targetTable)) {
        return OperationResult::failure("Target table is not available.");
    }

    SessionRecord *session = database.findSessionById(sourceTable->activeSessionId);
    if (session == nullptr || session->status == "CLOSED") {
        return OperationResult::failure("Session is invalid.");
    }
    if (session->status == "BILL_REQUESTED") {
        return OperationResult::failure("Cannot transfer table after bill has been requested.", "SESSION_LOCKED_FOR_BILLING", "REOPEN_BILL_FIRST");
    }

    session->tableIds.erase(std::remove(session->tableIds.begin(), session->tableIds.end(), sourceTable->id), session->tableIds.end());
    session->tableIds.push_back(targetTable->id);
    sourceTable->state = "CLEANING";
    sourceTable->activeSessionId = 0;
    targetTable->state = "OCCUPIED";
    targetTable->activeSessionId = session->id;

    database.addAudit(actor, "Transferred session #" + std::to_string(session->id) + " from " + sourceTable->code + " to " + targetTable->code);
    database.save();
    return OperationResult::success("Transferred to " + targetTable->code + ". Old table is CLEANING.", session->id);
}

OperationResult markTableCleaned(FileDatabase &database, const std::string &tableCode, const std::string &actor) {
    OperationResult permission = policy::requirePermission(database, actor, "table.cleaned");
    if (!permission.ok) {
        return permission;
    }

    TableRecord *table = database.findTableByCode(tableCode);
    if (table == nullptr || table->state != "CLEANING") {
        return OperationResult::failure("Table is not in CLEANING state.");
    }

    table->state = "AVAILABLE";
    database.addAudit(actor, "Marked table " + table->code + " as AVAILABLE");
    database.save();
    return OperationResult::success("Table is now AVAILABLE.");
}

}
