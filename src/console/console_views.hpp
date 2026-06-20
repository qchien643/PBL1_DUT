#pragma once

#include "../domain/models.hpp"
#include "../infrastructure/file_database.hpp"

#include <string>

namespace app::console {

void printMenuItems(FileDatabase &database, bool includeHidden = false);
void printTables(FileDatabase &database);
void printSessionOrders(FileDatabase &database, const SessionRecord &session);
void printPendingOrders(FileDatabase &database);
void printCancelRequests(FileDatabase &database);
void printOpenBills(FileDatabase &database);
void printKitchenTasks(FileDatabase &database, const std::string &station);
void printRecommendations(FileDatabase &database, const SessionRecord &session);

}
