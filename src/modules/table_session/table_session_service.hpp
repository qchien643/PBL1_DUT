#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <string>

namespace app::table_session {

std::string tableCodesForSession(FileDatabase &database, const SessionRecord &session);

OperationResult openTable(FileDatabase &database, const std::string &tableCode, const std::string &actor);
OperationResult mergeTables(FileDatabase &database, const std::string &mainTableCode, const std::string &joinedTableCode, const std::string &actor);
OperationResult transferTable(FileDatabase &database, const std::string &sourceCode, const std::string &targetCode, const std::string &actor);
OperationResult markTableCleaned(FileDatabase &database, const std::string &tableCode, const std::string &actor);

}
