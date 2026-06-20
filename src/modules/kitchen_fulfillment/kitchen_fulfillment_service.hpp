#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <string>
#include <vector>

namespace app::kitchen_fulfillment {

std::vector<KitchenTaskRecord> activeTasksForStation(const FileDatabase &database, const std::string &station);
OperationResult startTask(FileDatabase &database, int taskId, const std::string &station, const std::string &actor);
OperationResult completeTask(FileDatabase &database, int taskId, const std::string &station, const std::string &actor);
OperationResult markServed(FileDatabase &database, int taskId, const std::string &actor);
OperationResult reportIssue(FileDatabase &database, int taskId, const std::string &station, const std::string &reason, const std::string &actor);
OperationResult resolveIssue(FileDatabase &database, int issueId, const std::string &resolution, const std::string &actor);

}
