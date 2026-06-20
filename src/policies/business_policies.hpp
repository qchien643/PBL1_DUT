#pragma once

#include "../domain/models.hpp"
#include "../infrastructure/file_database.hpp"

namespace app::policy {

bool canOpenTable(const TableRecord &table);
bool canOrderItem(const MenuItemRecord &item);
bool canApproveOrder(const OrderRecord &order);
bool canRequestCancel(const OrderItemRecord &item);
bool canApproveCancel(const OrderItemRecord &item, const KitchenTaskRecord *task);
bool canRequestBill(const FileDatabase &database, const SessionRecord &session);

OperationResult requirePermission(const FileDatabase &database, const std::string &actor, const std::string &permissionKey);
OperationResult evaluateOrderableItem(const MenuItemRecord *item);
OperationResult evaluateCreateBill(const FileDatabase &database, const SessionRecord &session);
OperationResult evaluatePayment(const FileDatabase &database, const BillRecord &bill, int paidAmount, int requestBillVersion);
OperationResult evaluateKitchenTaskTransition(const KitchenTaskRecord *task, const std::string &station, const std::string &targetStatus);

}
