#pragma once

#include "../../domain/models.hpp"
#include "../../infrastructure/file_database.hpp"

#include <string>

namespace app::payment_billing {

int billTotal(const FileDatabase &database, int sessionId);
OperationResult createBill(FileDatabase &database, int sessionId, const std::string &actor);
OperationResult createBillForTable(FileDatabase &database, const std::string &tableCode, const std::string &actor);
OperationResult confirmPayment(FileDatabase &database, int billId, const std::string &paymentMethod, const std::string &actor);
OperationResult confirmPayment(FileDatabase &database, int billId, const std::string &paymentMethod, int paidAmount, int billVersion, const std::string &idempotencyKey, const std::string &actor);
OperationResult reopenBill(FileDatabase &database, int billId, const std::string &actor);

}
