# Commands And Interfaces - Payment Billing

| CMD command | Service method | Actor | Policy |
| --- | --- | --- | --- |
| `RequestBill` | `PaymentService.requestBill` | Customer/Staff | `PaymentPolicy`, `PricingPolicy` |
| `ViewBill` | `PaymentService.getBill` | Customer/Staff | `PermissionPolicy` |
| `ApplyManualDiscount` | `PaymentService.applyDiscount` | Manager/Cashier | `PermissionPolicy`, `PricingPolicy` |
| `ConfirmPayment` | `PaymentService.confirmPayment` | Cashier | `PaymentPolicy`, `AuditPolicy` |
| `CancelBill` | `PaymentService.cancelBill` | Manager | `PermissionPolicy` |

## Input/output

| Method | Input | Output |
| --- | --- | --- |
| `requestBill` | `sessionId` | `billId`, bill summary |
| `confirmPayment` | `billId`, `method`, `billVersion` | payment confirmation |

## Policy-aware service flow

```text
requestBill
→ PermissionPolicy
→ CanCreateBillPolicy
→ BillCalculationPolicy
→ create bill snapshot/version
→ BillLockPolicy marks session locked
→ audit + notify
```

```text
confirmPayment
→ PermissionPolicy
→ BillStalenessPolicy
→ CanPayBillPolicy
→ persist payment
→ close session/table cleaning
→ audit + notify
```
