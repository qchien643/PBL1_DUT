# Data Design - Payment Billing

## Tables

| Table | Purpose | Key fields |
| --- | --- | --- |
| `bills` | Bill theo session | `id`, `sessionId`, `status`, `subtotal`, `total`, `version` |
| `bill_lines` | Dòng bill | `billId`, `orderItemId`, `nameSnapshot`, `quantity`, `amount` |
| `bill_adjustments` | Discount/fee/override | `billId`, `type`, `amount`, `reason`, `actorId` |
| `payments` | Thanh toán | `id`, `billId`, `method`, `amount`, `status`, `confirmedBy` |
| `payment_status_history` | Lịch sử payment | `paymentId`, `fromStatus`, `toStatus`, `actorId` |

## Indexes

| Table | Index |
| --- | --- |
| `bills` | unique active `sessionId` |
| `payments` | `billId`, `status` |
| `bill_lines` | `billId`, `orderItemId` |

## Policy support data

| Data | Dùng bởi policy | Ghi chú |
| --- | --- | --- |
| `bills.status` | `CanCreateBillPolicy`, `CanPayBillPolicy` | `OPEN/STALE/VOIDED/PAID` |
| `bills.sessionVersion` | `BillStalenessPolicy` | So với current session version |
| `bill_lines.chargeableStatus` | `BillCalculationPolicy` | Snapshot vì item có thể đổi sau |
| `payments.method`, `paidAmount` | `CanPayBillPolicy` | Cash/card/bank transfer manual |
| `billing_adjustments.reason` | `ManagerOverridePolicy`, audit | Required nếu void/discount |
