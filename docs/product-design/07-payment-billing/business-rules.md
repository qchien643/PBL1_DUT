# Business Rules - Payment Billing

| Rule ID | Rule | Policy | Trạng thái |
| --- | --- | --- | --- |
| PAY_001 | Một session có một bill MVP | `PaymentPolicy` | Required |
| PAY_002 | Thanh toán cuối bữa | `PaymentPolicy` | Required |
| PAY_003 | Không request bill nếu còn task preparing | `PaymentPolicy` | Required |
| PAY_004 | Bill không tính `cancelled` order item | `PricingPolicy` | Required |
| PAY_005 | Cashier/manager mới confirm payment | `PermissionPolicy` | Required |
| PAY_006 | Bill paid khóa order/cancel mới | `PaymentPolicy` | Required |
| PAY_007 | Confirm payment phải audit | `AuditPolicy` | Required |

## Pricing MVP

| Component | Formula |
| --- | --- |
| `subtotal` | Sum non-cancelled item totals |
| `serviceCharge` | `(subtotal - discount) * percent` |
| `tax` | `(subtotal - discount + serviceCharge) * percent` |
| `total` | `subtotal - discount + serviceCharge + tax` |

## Policy contracts

| Policy | Business question | Input context | Deny codes | Audit/notification |
| --- | --- | --- | --- | --- |
| `CanCreateBillPolicy` | Session đã đủ ổn định để tạo bill chưa? | session, orders, tasks, cancel requests | `BILL_BLOCKED_BY_ACTIVE_WORK`, `BILL_ALREADY_EXISTS` | Audit required |
| `BillCalculationPolicy` | Item nào được tính tiền? | order item status, task state, adjustments | `BILL_CALCULATION_INVALID_STATE` | Audit if adjustment |
| `BillLockPolicy` | Bill open có khóa order mới không? | session, open bill | `SESSION_LOCKED_FOR_BILLING` | Notify table/cashier |
| `BillStalenessPolicy` | Bill snapshot còn đúng không? | bill version, session version | `BILL_STALE_RECALCULATE_REQUIRED` | Audit required |
| `CanPayBillPolicy` | Cashier có được confirm payment không? | bill, payment amount/method, actor | `BILL_NOT_OPEN`, `PAYMENT_AMOUNT_INVALID` | Audit required |

Billing không tự sửa order/kitchen; nếu có blocker thì trả blocker list.
