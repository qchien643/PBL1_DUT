#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <vector>

namespace app::reporting_audit {

int paidRevenue(const FileDatabase &database);
std::vector<AuditEventRecord> recentAuditEvents(const FileDatabase &database, int limit);

}
