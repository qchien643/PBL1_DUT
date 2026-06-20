# Audit And Notification Mapping

## 1. Audit Severity

| Severity | Khi nào dùng | Ví dụ |
|---|---|---|
| `LOW` | Hành động vận hành thường ngày | open table, submit order |
| `MEDIUM` | Đổi trạng thái ảnh hưởng nhiều actor | accept order, mark ready |
| `HIGH` | Ảnh hưởng tiền/chi phí/trách nhiệm | payment, manager void, kitchen issue |
| `CRITICAL` | Cấu hình/quyền/audit integrity | config changed, role changed, audit failure |

## 2. Action Mapping

| Action | Audit | Notification |
|---|---|---|
| `OpenTable` | `LOW`, required | customer table, cashier |
| `MergeTable` | `HIGH`, required | affected table screens, cashier |
| `TransferTable` | `MEDIUM`, required | old/new table screens, cashier |
| `UpdateConfig` | `CRITICAL`, required | manager/cashier if active config changed |
| `SubmitOrder` | `LOW`, required | cashier |
| `AcceptOrder` | `MEDIUM`, required | kitchen/bar, customer |
| `RejectOrder` | `MEDIUM`, required | customer |
| `UnavailableAtAccept` | `HIGH`, required | customer, cashier |
| `RequestCancelItem` | `MEDIUM`, required | cashier |
| `ApproveCancelItem` | `HIGH`, required | kitchen if task exists, customer |
| `ManagerVoidItem` | `HIGH`, required | cashier, customer if bill changed |
| `CreateKitchenTask` | `MEDIUM`, required | station screen |
| `StartKitchenTask` | `LOW`, optional | cashier/customer state refresh |
| `MarkTaskReady` | `MEDIUM`, required | waiter/cashier/customer |
| `ReportKitchenIssue` | `HIGH`, required | cashier/waiter/customer/manager |
| `CreateBill` | `HIGH`, required | customer/cashier |
| `ReopenBill` | `HIGH`, required | customer/cashier |
| `ConfirmPayment` | `HIGH`, required | manager/reporting/customer |
| `ChangeMenuAvailability` | `MEDIUM`, required | customer screens/cashier |
| `TrainRecommendationModel` | `MEDIUM`, optional | manager |
| `RoleChanged` | `CRITICAL`, required | affected staff/manager |

## 3. Notification Routing Rules

| Event | Recipient |
|---|---|
| `TABLE_OPENED` | `customer:{tableCode}`, `cashier` |
| `ORDER_SUBMITTED` | `cashier` |
| `ORDER_NEEDS_CUSTOMER_CONFIRMATION` | `customer:{tableCode}`, `cashier` |
| `ORDER_ACCEPTED` | `customer:{tableCode}`, `station:{stationId}` |
| `ORDER_ITEM_CANCELLED` | `customer:{tableCode}`, `cashier`, relevant station |
| `KITCHEN_TASK_CREATED` | `station:{stationId}` |
| `KITCHEN_TASK_READY` | `cashier`, `waiter`, `customer:{tableCode}` |
| `KITCHEN_ISSUE_REPORTED` | `cashier`, `waiter`, `manager`, `customer:{tableCode}` |
| `BILL_CREATED` | `customer:{tableCode}`, `cashier` |
| `BILL_STALE` | `cashier` |
| `PAYMENT_CONFIRMED` | `cashier`, `manager/reporting` |
| `MENU_ITEM_SOLD_OUT` | `customer screens`, `cashier` |

## 4. Audit Payload Chuẩn

```json
{
  "eventType": "ORDER_ITEM_CANCELLED",
  "severity": "HIGH",
  "actorId": "STAFF-001",
  "actorRole": "cashier",
  "resourceType": "orderItem",
  "resourceId": "OI-001",
  "reason": "Customer ordered wrong item",
  "before": {"status": "ACCEPTED"},
  "after": {"status": "CANCELLED"},
  "correlationId": "REQ-20260620-0001"
}
```

## 5. Nguyên Tắc

- Audit cho biết **ai làm gì, vì sao, trước/sau ra sao**.
- Notification cho biết **màn hình nào cần refresh hoặc hành động**.
- Audit không được phụ thuộc notification.
- Notification mất/lặp không làm sai dữ liệu chính.

