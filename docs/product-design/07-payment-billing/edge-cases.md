# Edge Cases - Payment Billing

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Request bill hai lần | Idempotent, trả bill hiện tại | Payment |
| Còn task preparing | Chặn request bill | Kitchen |
| Có cancel request pending | Yêu cầu xử lý cancel trước | Order |
| Bill changed before confirm | So sánh `billVersion`, yêu cầu reload | Payment |
| Confirm paid hai lần | Chặn nếu bill already paid | Audit |
| Order cancel sau paid | Không thuộc cancel flow; cần adjustment ngoài MVP | Reporting |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Còn order submitted/needs confirmation | `CanCreateBillPolicy`, `CustomerDecisionGatePolicy` | Deny bill, yêu cầu xử lý order trước | Optional/Required tùy blocker | Cashier/customer |
| Còn task pending/preparing/issue | `KitchenCompletionGatePolicy` | Deny bill, liệt kê món đang xử lý | Optional/Required for issue | Cashier/waiter |
| Bill open rồi khách gọi thêm | `BillLockPolicy`, `ReopenBillPolicy` | Void bill open, reopen session | Required | Table/cashier |
| Pay bill stale | `BillStalenessPolicy` | Deny payment, yêu cầu tính lại | Required | Cashier |
| Payment thiếu tiền | `CanPayBillPolicy` | Deny, bill giữ `OPEN` | No | Cashier |
