# Edge Cases - Kitchen Fulfillment

| Edge case | Xử lý | Ảnh hưởng |
| --- | --- | --- |
| Category chưa có station | Chặn accept order | Order |
| Order có food và drink | Tạo task riêng theo station | Kitchen |
| Item cancelled khi task pending | TaskItem cancelled, không hiện bếp | Order/Kitchen |
| Kitchen start task stale | Re-read task status, trả lỗi | Kitchen CMD |
| Bếp báo hết món sau accepted | Tạo issue, staff xử lý hủy/đổi món | Notification |
| Một task ready trước task khác | Staff giao từng món, bill vẫn theo item | Fulfillment |

## Policy-backed resolution

| Edge case | Policy | Resolution | Audit | Notification |
| --- | --- | --- | --- | --- |
| Item thiếu station | `KitchenRoutingPolicy` | Chặn fulfillment, yêu cầu sửa cấu hình | Required | Manager/cashier |
| Ready khi chưa preparing | `KitchenTaskStatePolicy` | Deny, task giữ nguyên | Warning | Kitchen |
| Cancel/start race | `CancelKitchenRacePolicy` | Reload state mới nhất, quyết định cancel hoặc preparing | Required | Cashier/kitchen |
| Bếp báo issue | `KitchenIssuePolicy` | Task `ISSUE`, item `ISSUE_PENDING_DECISION`, block bill | Required | Cashier/customer/manager |
| READY chưa SERVED | `ReadyToServedPolicy` | Block bill hoặc dùng flag MVP `ReadyCountsAsServedPolicy` | Optional | Waiter/cashier |
