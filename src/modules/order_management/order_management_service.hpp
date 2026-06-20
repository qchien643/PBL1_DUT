#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <utility>
#include <vector>

namespace app::order_management {

using CartLine = std::pair<int, int>;

OperationResult submitOrder(FileDatabase &database, int sessionId, const std::vector<CartLine> &cart, const std::string &actor);
OperationResult submitOrder(FileDatabase &database, int sessionId, const std::vector<CartLine> &cart, const std::string &actor, const std::string &idempotencyKey);
OperationResult acceptOrder(FileDatabase &database, int orderId, const std::string &actor);
OperationResult rejectOrder(FileDatabase &database, int orderId, const std::string &actor);
OperationResult resolveCustomerDecision(FileDatabase &database, int orderId, const std::string &decision, const std::vector<CartLine> &replacements, const std::string &actor);
OperationResult requestCancel(FileDatabase &database, int sessionId, int orderItemId, const std::string &actor);
OperationResult approveCancel(FileDatabase &database, int orderItemId, const std::string &actor);
void markOrderCompletedIfReady(FileDatabase &database, int orderId);

}
