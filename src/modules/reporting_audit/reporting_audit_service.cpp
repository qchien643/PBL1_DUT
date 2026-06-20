#include "reporting_audit_service.hpp"

namespace app::reporting_audit {

int paidRevenue(const FileDatabase &database) {
    int total = 0;
    for (const BillRecord &bill : database.bills) {
        if (bill.status == "PAID") {
            total += bill.total;
        }
    }
    return total;
}

std::vector<AuditEventRecord> recentAuditEvents(const FileDatabase &database, int limit) {
    std::vector<AuditEventRecord> events;
    const int count = static_cast<int>(database.auditEvents.size());
    const int start = count > limit ? count - limit : 0;
    for (int index = start; index < count; ++index) {
        events.push_back(database.auditEvents[index]);
    }
    return events;
}

}
